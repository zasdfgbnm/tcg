#pragma once

#include <optional>
#include <string>
#include <map>
#include <vector>

constexpr const char *cgroup_procs = "/sys/fs/cgroup/cgroup.procs";
constexpr const char *root_dir = "/sys/fs/cgroup/terminals";

std::string user_dir();
std::string name_dir(std::string name,
                     std::optional<bool> assert_existence = std::nullopt);

struct HelpInfo {
  std::string description = "";
};

class RegisterHelpInfo {
  static std::map<std::string, HelpInfo> registry;
public:
  RegisterHelpInfo(std::string name, const HelpInfo &info) {
      registry[name] = info;
  }
};
