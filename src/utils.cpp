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

std::map<std::string, Command> RegisterCommand::cmd_registry;
std::map<std::string, std::string> RegisterCommand::alias_registry;

RegisterCommand::RegisterCommand(const std::string &name, const Command &info) {
  cmd_registry[name] = info;
  cmd_registry[name].name = name;
  alias_registry[name] = name;
  for (auto &i : info.alias) {
    if (alias_registry.find(i) != alias_registry.end()) {
      throw std::runtime_error("Conflicting alias. Please report a bug at: https://github.com/zasdfgbnm/tcg");
    }
    alias_registry[i] = name;
  }
}
