#pragma once

#include "windefs.h"
#include <cstdint>
#include <stdexcept>

namespace process
{
  std::uint32_t find(const wchar_t* proc);

  bool attach(std::uint32_t pid);
  void detach();

  bool grant_handle_access(HANDLE handle, ACCESS_MASK access_rights);

  bool read(PVOID base, PVOID buf, size_t len);
  bool write(PVOID base, PVOID buf, size_t len);

  template<typename T, typename U>
  T read(U base)
  {
    T temp = T{};
    read((PVOID)base, &temp, sizeof(T));
    return temp;
  }
  template<typename T, typename U>
  bool write(U base, T value)
  {
    return write((PVOID)base, &value, sizeof(T));
  }
};
