#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>

enum supported_versions
{
  win7_sp1     = 0x060101,
  win8         = 0x060200,
  win81        = 0x060300,
  win10        = 0x0A0000,
  win10_cu     = 0x0A0002
};

class unsupported_version
  : public std::exception
{
public:
  unsupported_version(std::uint32_t major, std::uint32_t minor, std::uint32_t sp, std::uint32_t build)
  {
    char buf[128];
    sprintf_s(buf, "Unsupported OS build: %d.%d.%d.%d", major, minor, sp, build);
    message = std::string(buf, strlen(buf));
  }

  const char* what() const override
  {
    return message.data();
  }

private:
  std::string message;
};

class unsupported_processor
  : public std::exception
{
public:
  unsupported_processor(const char* vendor)
  {
    char buf[128];
    sprintf_s(buf, "Unsupported processor (Vendor: %s)", vendor);
    message = std::string(buf, strlen(buf));
  }

  const char* what() const override
  {
    return message.data();
  }

private:
  std::string message;
};

namespace dyn_data
{
  void ensure_intel_cpu();
  void LoadCpuz();

  extern std::uint32_t os_version;
  extern std::uint32_t offset_directorytable;
  extern std::uint32_t offset_process_id;
  extern std::uint32_t offset_process_links;
  extern std::uint32_t offset_object_table;
}
