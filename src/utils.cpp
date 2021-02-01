#include "utils.hpp"
#include <boost/filesystem.hpp>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

namespace fs = boost::filesystem;

extern bool is_sandbox;

std::string user_dir() {
  if (is_sandbox) {
    return "/";
  }
  auto uid = getuid();
  return fmt::format("{}/{}/", root_dir, uid);
}

std::string name_dir(const std::string &name,
                     std::optional<bool> assert_existence) {
  auto logger = spdlog::get("utils");
  logger->debug("Getting directory for {}...", name);
  auto dir = user_dir() + name;
  logger->debug("The directory should be {}.", dir);
  if (assert_existence.has_value()) {
    bool v = assert_existence.value();
    logger->debug("Check that the directory{} exist.", (v ? "" : " does not"));
    if (v && !fs::is_directory(dir)) {
      logger->error("Cgroup does not exist.");
      exit(EXIT_FAILURE);
    }
    if (!v && fs::is_directory(dir)) {
      logger->error("Cgroup name already used.");
      exit(EXIT_FAILURE);
    }
    logger->debug("Directory existence check pass.");
  } else {
    logger->debug("Do not check directory existence.");
  }
  return dir;
}

std::map<std::string, const Command *> Command::registry;

void invalid_argument();

void handler::call(const char *args[]) const {
  assert(args[num_arg_] == nullptr);
  switch (num_arg_) {
  case 0:
    f0();
    return;
  case 1:
    f1(args[0]);
    return;
  case 2:
    f2(args[0], args[1]);
    return;
  case 3:
    f3(args[0], args[1], args[2]);
    return;
  default:
    invalid_argument();
  }
}

Command::Command(const std::string &name, const std::vector<std::string> &alias,
                 const std::string &short_description,
                 const std::string &long_description,
                 const std::vector<handler> &handlers, bool sandbox)
    : name(name), alias(alias), short_description(short_description),
      long_description(long_description), handlers(handlers), sandbox(sandbox) {
  if (registry.find(name) != registry.end()) {
    throw std::runtime_error(
        std::string("Conflicting name. Please report a bug at: ") + url);
  }
  registry[name] = this;
  for (auto &a : alias) {
    if (registry.find(a) != registry.end()) {
      throw std::runtime_error(
          std::string("Conflicting alias. Please report a bug at: ") + url);
    }
    registry[a] = this;
  }
}

class UndefinedCommand final : public Command {
public:
  UndefinedCommand() : Command({}, {}, {}, {}, {}) {}
  bool defined() const override { return false; }
} undefined_command;

const Command *Command::get(const std::string &name) {
  auto i = registry.find(name);
  if (i == registry.end()) {
    return &undefined_command;
  }
  return registry[name];
}

void Command::call(const char *args[]) const {
  uint8_t num_arg = 0;
  while (args[num_arg] != nullptr)
    num_arg++;
  for (auto &h : handlers) {
    if (h.num_arg() == num_arg) {
      h.call(args);
      return;
    }
  }
  invalid_argument();
}
