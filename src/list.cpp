#include <iostream>
#include <string>
#include <vector>

#include <fmt/color.h>
#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <boost/filesystem.hpp>

#include <unistd.h>

#include "utils.hpp"

namespace fs = boost::filesystem;

bool stdout_is_tty() {
  return isatty(fileno(stdout));
}

void list() {
  auto logger = spdlog::get("list");
  logger->info("List all existing cgroups.");
  bool tty = stdout_is_tty();
  logger->info("The stdout {} a tty, {} color.", (tty ? "is" : "is not"), (tty ? "enable" : "disable"));
  fmt::text_style cg_style;
  if (tty) {
    cg_style = fg(fmt::color::blue) | fmt::emphasis::bold;
  }
  auto r = root_dir();
  logger->debug("Root directory is {}, iterating it.", r);
  fs::path p(r);
  fs::recursive_directory_iterator end;
  for (fs::recursive_directory_iterator i(p); i != end; ++i) {
    if (fs::is_directory(*i)) {
      auto cg = i->path().filename().string();
      logger->debug("Found cgroup {}.", cg);
      fmt::print(cg_style, cg);
      fmt::print("\n");
    }
  }
}