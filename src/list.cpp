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
  logger->debug("List all used names.");
  fs::path p(root_dir());
  fs::recursive_directory_iterator end;
  for (fs::recursive_directory_iterator i(p); i != end; ++i) {
    if (fs::is_directory(*i)) {
      fmt::print(i->path().filename().string());
      fmt::print(" ");
    }
  }
}