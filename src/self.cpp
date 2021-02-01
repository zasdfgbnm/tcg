#include <fstream>
#include <string>
#include <unistd.h>

#include <boost/filesystem.hpp>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "utils.hpp"

static RegisterCommand _({.name = "self",
                          .alias = {"sf"},
                          .sandbox = false, // disable sandbox to access /proc
                          .short_description = "TODO",
                          .long_description = R"body(TODO)body"});

namespace fs = boost::filesystem;

void self() {
  auto logger = spdlog::get("self");
  logger->info("Getting the cgroup for the current shell");
  auto pid = getpid();
  auto cg = fmt::format("/proc/{}/cgroup", pid);
  if (!fs::exists(fs::path(cg))) {
    logger->info("Procfs not mounted, or cgroup not enabled");
    exit(EXIT_FAILURE);
  }
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
