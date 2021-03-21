#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <fmt/color.h>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "command.hpp"
#include "utils.hpp"

namespace list {

namespace fs = boost::filesystem;

fmt::text_style cg_style =
    maybe_style(fg(fmt::color::green) | fmt::emphasis::bold);

void print_procs(std::shared_ptr<spdlog::logger> logger,
                 const std::string &name) {
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

Command command(/*name =*/"list",
                /*alias =*/{"l", "ls"},
                /*short_description =*/"list current cgroups",
                /*additional_note =*/"",
                /*sandbox =*/false // disable sandbox to access /proc
);

DEFINE_HANDLER({}, "print a table of existing cgroups and its details", {
  auto logger = spdlog::get("list");
  logger->info("List all existing cgroups.");
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
});

DEFINE_HANDLER({"cgroups"_kwd->alias("cgs")}, "list existing cgroups", {
  auto logger = spdlog::get("list");
  logger->info("List all existing cgroups.");
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
      fmt::print(cg);
      fmt::print("\n");
    }
  }
});

} // namespace list
