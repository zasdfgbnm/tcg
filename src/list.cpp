#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <fmt/color.h>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <unistd.h>

#include "utils.hpp"

namespace fs = boost::filesystem;

bool stdout_is_tty() { return isatty(fileno(stdout)); }

void print_procs(std::shared_ptr<spdlog::logger> logger, std::string name) {
  auto procs_file = name_dir(name, true) + "/cgroup.procs";
  logger->debug("Reading process list from {}.", procs_file);
  std::ifstream in(procs_file);
  pid_t pid;
  std::string cmd;
  bool first = true;
  while (in >> pid) {
    logger->debug("Get pid {} from cgroup.procs", pid);
    if (!first) {
      fmt::print(" ");
    }
    first = false;
    auto cmd_file = fmt::format("/proc/{}/cmdline", pid);
    logger->debug("Reading {} to get program command line.", cmd_file);
    std::ifstream cmdin(cmd_file);
    cmdin >> cmd;
    logger->debug("Command line of pid {} is {}.", pid, cmd);
    std::vector<std::string> results;
    boost::split(results, cmd, [](char c) { return c == '\0'; });
    fmt::print("{}", results[0]);
  }
}

void list() {
  auto logger = spdlog::get("list");
  logger->info("List all existing cgroups.");
  bool tty = stdout_is_tty();
  logger->info("The stdout {} a tty, {} color.", (tty ? "is" : "is not"),
               (tty ? "enable" : "disable"));
  fmt::text_style cg_style;
  if (tty) {
    cg_style = fg(fmt::color::green) | fmt::emphasis::bold;
  }
  auto r = user_dir();
  logger->debug("Root directory is {}, iterating it.", r);
  fs::path p(r);
  if (!fs::exists(p)) {
    logger->info("Root directory does not exist, showing empty list.");
    return;
  }
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
