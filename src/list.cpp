#include <iostream>
#include <string>
#include <vector>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <boost/filesystem.hpp>

#include "utils.hpp"

namespace fs = boost::filesystem;

void list() {
  auto logger = spdlog::get("list");
  logger->info("List all existing cgroups.");
  auto r = root_dir();
  logger->debug("Root directory is {}, iterating it.", r);
  fs::path p(r);
  fs::recursive_directory_iterator end;
  for (fs::recursive_directory_iterator i(p); i != end; ++i) {
    if (fs::is_directory(*i)) {
      auto cg = i->path().filename().string();
      logger->debug("Found cgroup {}.", cg);
      fmt::print(cg);
      fmt::print(" ");
    }
  }
}