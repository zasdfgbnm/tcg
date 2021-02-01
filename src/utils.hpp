#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

constexpr const char *cgroup_procs = "/sys/fs/cgroup/cgroup.procs";
constexpr const char *root_dir = "/sys/fs/cgroup/terminals";

std::string user_dir();
std::string name_dir(const std::string &name,
                     std::optional<bool> assert_existence = std::nullopt);

struct HelpInfo {
  std::string description;
  std::string body;

  class reg {
    static std::map<std::string, HelpInfo> registry;
    friend class HelpInfo;

  public:
    reg(std::string name, const HelpInfo &info) { registry[name] = info; }
  };

  static const HelpInfo &get(const std::string &name) {
    return reg::registry[name];
  }

  static const std::map<std::string, HelpInfo> &all() { return reg::registry; }
};

#define RegisterHelpInfo(...) static HelpInfo::reg ister(__VA_ARGS__)
