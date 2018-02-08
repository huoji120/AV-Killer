#include "dyn_data.hpp"
#include "windefs.h"
#include <stdexcept>

namespace dyn_data
{
  std::uint32_t os_version;
  std::uint32_t offset_directorytable;
  std::uint32_t offset_process_id;
  std::uint32_t offset_process_links;
  std::uint32_t offset_object_table;

  typedef NTSTATUS(NTAPI* RtlGetVersion_t)(
    _Out_ PRTL_OSVERSIONINFOW lpVersionInformation
  );

  void ensure_intel_cpu()
  {
    char buf[13] = "";
    int reg[4];
    __cpuid(reg, 0);
    memcpy(buf, &reg[1], 4);
    memcpy(buf + 4, &reg[3], 4);
    memcpy(buf + 8, &reg[2], 4);
    buf[12] = 0;

    if(strcmp("GenuineIntel", buf) != 0)
      throw unsupported_processor(buf);
  }

  void LoadCpuz()
  {
    static auto RtlGetVersion = (RtlGetVersion_t)GetProcAddress(GetModuleHandle(TEXT("NTDLL")), "RtlGetVersion");
    
    auto osvi = OSVERSIONINFOEXW{ sizeof(OSVERSIONINFOEXW) };

    RtlGetVersion((POSVERSIONINFOW)&osvi);
    
    auto version_long = (osvi.dwMajorVersion << 16) | (osvi.dwMinorVersion << 8) | osvi.wServicePackMajor;
    
    switch(version_long) {
    case win7_sp1:
      os_version = win7_sp1;
      offset_directorytable = 0x028;
      offset_process_id     = 0x180;
      offset_process_links  = 0x188;
      offset_object_table   = 0x200;
      break;
    case win8:
      os_version = win8;
      offset_directorytable = 0x028;
      offset_process_id     = 0x2e0;
      offset_process_links  = 0x2e8;
      offset_object_table   = 0x408;
      break;
    case win81:
      os_version = win81;
      offset_directorytable = 0x028;
      offset_process_id     = 0x2e0;
      offset_process_links  = 0x2e8;
      offset_object_table   = 0x408;
      break;
    case win10:
    {
      switch(osvi.dwBuildNumber) {
      case 10240:
      case 10586:
      case 14393:
        os_version = win10;
        offset_directorytable = 0x028;
        offset_process_id     = 0x2E8;
        offset_process_links  = 0x2F0;
        offset_object_table   = 0x418;
        break;
      case 15063:
        os_version = win10_cu;
        offset_directorytable = 0x028;
        offset_process_id     = 0x2E0;
        offset_process_links  = 0x2E8;
        offset_object_table   = 0x418;
        break;
      default:
        throw unsupported_version(osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.wServicePackMajor, osvi.dwBuildNumber);
      }
      break;
    }
    default:
    {
      throw unsupported_version(osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.wServicePackMajor, osvi.dwBuildNumber);
    }
    }
  }
}