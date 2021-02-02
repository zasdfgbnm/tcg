#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_set>
#include <vector>

#include <boost/filesystem.hpp>
#include <fmt/os.h>
#include <spdlog/spdlog.h>

#include "command.hpp"
#include "utils.hpp"

#ifdef __linux__

#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace fs = boost::filesystem;

std::unordered_set<std::string> names = {
#include "names.txt"
};

void validate_name(std::shared_ptr<spdlog::logger> logger,
                   const std::string &name) {
  for (char c : name) {
    if (c == '_' || (c <= 'z' && c >= 'a') || (c <= 'Z' && c >= 'A') ||
        (c <= '9' && c >= '0')) {
      continue;
    }
    logger->error("Illegal name: can only use letters, digits, or underscore.");
    exit(EXIT_FAILURE);
  }
}

bool is_used(const std::string &name) {
  auto logger = spdlog::get("utils");
  auto d = user_dir() + name;
  logger->debug(
      "Check if name {}, which correspond to directory {} is already used.",
      name, d);
  fs::path p(d);
  return fs::is_directory(p);
}

std::string new_name() {
  auto logger = spdlog::get("utils");
  logger->debug("Getting a new builtin name...");
  for (auto n : names) {
    logger->debug("Trying {}...", n);
    if (!is_used(n)) {
      logger->debug("Name {} is available, pick it.", n);
      return n;
    }
    logger->debug("Name {} is already used.", n);
  }
  logger->error("Run out of names.");
  exit(EXIT_FAILURE);
}

void create1(const std::string &name_) {
  auto logger = spdlog::get("create");
  logger->info("Start creating a new cgroup");
  std::string name;
  if (name_ == "") {
    name = new_name();
    logger->info("Name not specified, use builtin name {}", name);
  } else {
    logger->info("Name specified as {}, will validating.", name_);
    validate_name(logger, name_);
    logger->info("Name pass validation", name_);
    name = name_;
  }
  std::cout << name << std::endl;

  // create new cgroup
  auto dir = name_dir(name, false);
  logger->info("Will create directory {}", dir);
  fs::create_directory(dir);
  logger->info("Done creating directory");
  auto procs_file = dir + "/cgroup.procs";
  auto events_file = dir + "/cgroup.events";

  // add parent process to the new cgroup
  logger->info("Adding parent process to the new cgroup");
  auto out = fmt::output_file(procs_file);
  auto ppid = getppid();
  logger->info("Parent process's pid is {}, will add to file {}", ppid,
               procs_file);
  out.print("{:d}", ppid);
  out.close();
  logger->info("Done adding parent process to the new cgroup");

  // detach from shell
  // See: http://netzmafia.de/skripten/unix/linux-daemon-howto.html
  logger->info("Will detach from shell and run in background to garbage "
               "collect the cgroup when it's empty.");
  auto pid = fork();
  if (pid < 0) {
    logger->critical("Fork failed.");
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    logger->debug("Fork succeed, parent process will exit.");
    exit(EXIT_SUCCESS);
  }
  logger->debug("In child process.");
  logger->debug("Setting file mask");
  umask(0);
  if (setsid() < 0) {
    logger->critical("Failed to get a new session id.");
    exit(EXIT_FAILURE);
  }
  if (chdir("/") < 0) {
    logger->critical("Failed to change directory to /");
    exit(EXIT_FAILURE);
  }
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  logger->info("Process successfully detached from shell.");

  // wait for cleanup
  // https://man7.org/tlpi/code/online/dist/inotify/demo_inotify.c.html
  // https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html#un-populated-notification
  logger->info("Waiting for cgroup cleanup");
  constexpr auto event_size = sizeof(inotify_event);
  constexpr auto buf_size = 1024 * (event_size + 16);
  std::vector<char> buf(buf_size);
  auto fd = inotify_init();
  if (fd < 0) {
    logger->critical("Unable to initialize inotify.");
    exit(EXIT_FAILURE);
  }
  if (inotify_add_watch(fd, events_file.c_str(), IN_MODIFY) < 0) {
    logger->critical("Unable to watch {}", events_file);
    exit(EXIT_FAILURE);
  }
  while (true) {
    int populated;
    std::string key;
    std::ifstream in(events_file);
    do {
      in >> key;
    } while (key != "populated");
    in >> populated;
    in.close();
    if (!populated) {
      logger->info("Cgroup {} is now empty, will remove it.", name);
      fs::remove(fs::path(dir));
      logger->info("Done removing {}. Exit.", dir);
      exit(EXIT_SUCCESS);
    }
    logger->info("Cgroup {} is not empty, continue waiting...", name);
    if (read(fd, buf.data(), buf_size) <= 0) {
      logger->critical("Failed to get event.");
      exit(EXIT_FAILURE);
    }
  }
}

void create0() { create1(""); }

#else

void create0() {}
void create1(const std::string &) {}

#endif

static Command command(
    /*name =*/"create",
    /*alias =*/{"c"},
    /*short_description =*/"create a new cgroup containing the current shell",
    /*long_description =*/R"body(
This command will create a new cgroup and add the current shell to it. TODO)body",
    /*handlers =*/{create0, create1});
