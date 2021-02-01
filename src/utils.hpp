#pragma once

#include <map>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

constexpr const char *url = "https://github.com/zasdfgbnm/tcg";
constexpr const char *cgroup_procs = "/sys/fs/cgroup/cgroup.procs";
constexpr const char *root_dir = "/sys/fs/cgroup/terminals";

std::string user_dir();
std::string name_dir(const std::string &name,
                     std::optional<bool> assert_existence = std::nullopt);

class handler {
  uint8_t num_arg_;
  using f0_t = void (*)();
  using f1_t = void (*)(const std::string &);
  using f2_t = void (*)(const std::string &, const std::string &);
  using f3_t = void (*)(const std::string &, const std::string &,
                        const std::string &);
  union {
    f0_t f0;
    f1_t f1;
    f2_t f2;
    f3_t f3;
  };

public:
  handler(f0_t f0) : num_arg_(0), f0(f0) {}
  handler(f1_t f1) : num_arg_(1), f1(f1) {}
  handler(f2_t f2) : num_arg_(2), f2(f2) {}
  handler(f3_t f3) : num_arg_(3), f3(f3) {}
  uint8_t num_arg() const { return num_arg_; }
  void call(const char *args[]) const;
};

class Command {
  static std::map<std::string, const Command *> registry;

public:
  std::string name;
  std::vector<std::string> alias;
  std::string short_description;
  std::string long_description;
  std::vector<handler> handlers;
  bool sandbox = true;

  Command(const std::string &name, const std::vector<std::string> &alias,
          const std::string &short_description,
          const std::string &long_description,
          const std::vector<handler> &handlers, bool sandbox = true);

  virtual bool defined() const { return true; }

  static const Command *get(const std::string &name);

  static const std::map<std::string, const Command *> &all() {
    return registry;
  }

  void call(const char *args[]) const;
};
