#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>
#include "../../windefs.h"

class cpuz_driver
{
  cpuz_driver();
  ~cpuz_driver();

  cpuz_driver(const cpuz_driver&)            = delete;
  cpuz_driver(cpuz_driver&&)                 = delete;
  cpuz_driver& operator=(const cpuz_driver&) = delete;
  cpuz_driver& operator=(cpuz_driver&&)      = delete;

public:
  static cpuz_driver& instance();

  bool ensure_loaded();
  bool load();
  bool unload();

  void unload_on_exit(bool enable) { unload_ = enable; }

  /**
   *  \brief Read some control registers
   */
  std::uint64_t read_cr0();
  std::uint64_t read_cr2();
  std::uint64_t read_cr3();

  /**
   *  \brief Translates a linear (virtual) address into it's respective physical address.
   *
   *  \param[in] directoryTableBase  The directory base for the process that owns the address.
   *  \param[in] virtualAddress      The virtual address that you want to translate. 
   *  
   *  \returns If the function succeeds, the return value is the physical address.
   *           If the function fails, the return value is 0.
   */
  std::uint64_t translate_linear_address(std::uint64_t directoryTableBase, LPVOID virtualAddress);
  
  /**
   *  \brief Reads from physical memory.
   */
  bool read_physical_address(std::uint64_t address, LPVOID buf, size_t len);

  /**
   *  \brief Writes to physical memory.
   */
  bool write_physical_address(std::uint64_t address, LPVOID buf, size_t len);
  
  /**
   *  \brief Reads from system memory. This is just a wrapper around read_physical_address
   */
  bool read_system_address(LPVOID address, LPVOID buf, size_t len);
    
  /**
   *  \brief Writes to system memory. This is just a wrapper around write_physical_address
   */
  bool write_system_address(LPVOID address, LPVOID buf, size_t len);


  template<typename T, typename U>
  T read_physical_address(U address)
  {
    T buf;

    if(!read_physical_address((std::uint64_t)address, (uint8_t*)&buf, sizeof(T)))
      throw std::runtime_error{ "Read failed" };

    return buf;
  }

  template<typename T, typename U>
  T read_system_address(U address)
  {
    T buf;

    if(!read_system_address((LPVOID)address, (uint8_t*)&buf, sizeof(T)))
      throw std::runtime_error{ "Read failed" };

    return buf;
  }

  template<typename T, typename U>
  bool write_physical_address(T address, U value)
  {
    return write_physical_address((LPVOID)address, (uint8_t*)&value, sizeof(U));
  }

  template<typename T, typename U>
  bool write_system_address(T address, U value)
  {
    return write_system_address((LPVOID)address, (uint8_t*)&value, sizeof(U));
  }
private:
  bool is_loaded();

  HANDLE serviceHandle_;
  HANDLE deviceHandle_;
  bool   unload_;
};