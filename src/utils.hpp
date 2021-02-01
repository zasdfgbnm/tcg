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

class Command;

class RegisterCommand {
  static std::map<std::string, Command> cmd_registry;
  static std::map<std::string, std::string> alias_registry;
  friend class Command;

public:
  RegisterCommand(const std::string &name, const Command &info);
};

struct Command {
  std::string name;
  std::vector<std::string> alias;
  std::string description;
  std::string body;

  static const Command &get(const std::string &name) {
    return RegisterCommand::cmd_registry[RegisterCommand::alias_registry[name]];
  }

  static const std::map<std::string, Command> &all() { return RegisterCommand::cmd_registry; }
};
