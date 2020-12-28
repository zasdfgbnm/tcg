#include <string>
#include <unistd.h>
#include <fstream>

#include <spdlog/spdlog.h>
#include <fmt/core.h>

#include "utils.hpp"

void self() {
  auto logger = spdlog::get("self");
  logger->info("Getting the cgroup for the current shell");
  auto pid = getpid();
  auto cg = fmt::format("/proc/{}/cgroup", pid);
  logger->debug("Reading {}...", cg);
  auto uid = getuid();
  auto userdir = fmt::format("/terminals/{}/", uid);
  std::string line;
  std::ifstream in(cg);
  while (std::getline(in, line)) {
    logger->debug("Parsing line: {}", line);
    std::size_t pos;
    if ((pos = line.find(userdir)) != std::string::npos) {
      fmt::print(line.substr(pos + userdir.size()) + "\n");
      break;
    }
  }
}