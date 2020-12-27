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

void print_procs(std::shared_ptr<spdlog::logger> logger, std::string name) {
  auto procs_file = name_dir(name, true) + "/cgroup.procs";
  logger->debug("Reading process list from {}.", procs_file);
  std::ifstream in(procs_file);
  pid_t pid;
  std::string cmd;
  bool first = true;
  while (in >> pid) {
    if (!first) {
      fmt::print(" ");
    }
    first = false;
    std::ifstream cmdin(fmt::format("/proc/{}/cmdline", pid));
    cmdin >> cmd;
    fmt::print("{}", cmd);
  }
}

void list() {
  auto logger = spdlog::get("list");
  logger->info("List all existing cgroups.");
  bool tty = stdout_is_tty();
  logger->info("The stdout {} a tty, {} color.", (tty ? "is" : "is not"), (tty ? "enable" : "disable"));
  fmt::text_style cg_style;
  if (tty) {
    cg_style = fg(fmt::color::red) | fmt::emphasis::bold;
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
      fmt::print("\t");
      print_procs(logger, cg);
      fmt::print("\n");
    }
  }
}