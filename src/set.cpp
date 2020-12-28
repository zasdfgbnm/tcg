#include <string>

#include <spdlog/spdlog.h>

#include "utils.hpp"

void set(std::string name, std::string key, std::string value) {
  auto logger = spdlog::get("set");
  logger->info("Setting cgroup {}'s {} to {}...", name, key, value);
  auto d = name_dir(name, true) + "/" + key;
  logger->debug("Writing {} to {}...", value, d);
}
