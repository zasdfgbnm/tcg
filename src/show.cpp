#include <fstream>
#include <string>

#include <fmt/os.h>
#include <spdlog/spdlog.h>

#include "utils.hpp"

void show(std::string name, std::string key) {
  auto logger = spdlog::get("show");
  logger->info("Showing cgroup {}'s {}...", name, key);
  auto d = name_dir(name, true) + "/" + key;
  logger->debug("Reading {}...", d);
  std::ifstream in(d);
  std::string line;
  while (std::getline(in, line)) {
    fmt::print("{}\n", line);
  }
  logger->debug("Done printting.");
}
