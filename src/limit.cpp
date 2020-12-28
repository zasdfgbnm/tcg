#include <string>

#include <spdlog/spdlog.h>

void limit_cpu(std::string name, std::string value) {
  // TODO
}

void limit(std::string name, std::string resource, std::string value) {
  auto logger = spdlog::get("limit");
  logger->info("Limiting cgroup {}'s {} usage to {}...", name, resource, value);
  if (resource == "cpu") {
    limit_cpu(name, value);
  } else {
    logger->critical("Unknown/unsupported resource {}.", resource);
  }
}