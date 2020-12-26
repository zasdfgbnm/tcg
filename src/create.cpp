#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <fmt/os.h>
#include <spdlog/spdlog.h>

#include <sys/inotify.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "utils.hpp"

namespace fs = boost::filesystem;

void validate_name(const std::string &name) {
  for (char c : name) {
    if (c == '_' || (c <= 'z' && c >= 'a') || (c <= 'Z' && c >= 'A') ||
        (c <= '9' && c >= '0')) {
      continue;
    }
    spdlog::error(
        "Illegal name: can only use letters, digits, or underscore.");
    exit(EXIT_FAILURE);
  }
}

void create(std::string name_) {
  spdlog::info("Start creating a new cgroup");
  std::string name;
  if (name_ == "") {
    name = new_name();
    spdlog::info("Name not specified, use builtin name {}", name);
  } else {
    spdlog::info("Name specified as {}, will validating.", name_);
    validate_name(name_);
    spdlog::info("Name pass validation", name_);
    name = name_;
  }
  std::cout << name << std::endl;

  // create new cgroup
  auto dir = name_dir(name, false);
  spdlog::info("Will create directory {}", dir);
  fs::create_directory(dir);
  spdlog::info("Done creating directory");
  auto procs_file = dir + "/cgroup.procs";
  auto events_file = dir + "/cgroup.events";

  // add parent process to the new cgroup
  spdlog::info("Adding parent process to the new cgroup");
  auto out = fmt::output_file(procs_file);
  auto ppid = getppid();
  spdlog::info("Parent process's pid is {}, will add to file {}", ppid,
               procs_file);
  out.print("{:d}", ppid);
  out.close();
  spdlog::info("Done adding parent process to the new cgroup");

  // detach from shell
  // See: http://netzmafia.de/skripten/unix/linux-daemon-howto.html
  spdlog::info("Will detach from shell and run in background to garbage "
               "collect the cgroup when it's empty.");
  auto pid = fork();
  if (pid < 0) {
    spdlog::critical("Fork failed.");
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    spdlog::debug("Fork succeed, parent process will exit.");
    exit(EXIT_SUCCESS);
  }
  spdlog::debug("In child process.");
  spdlog::debug("Setting file mask");
  umask(0);
  if (setsid() < 0) {
    spdlog::critical("Failed to get a new session id.");
    exit(EXIT_FAILURE);
  }
  if (chdir("/") < 0) {
    spdlog::critical("Failed to change directory to /");
    exit(EXIT_FAILURE);
  }
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);
  spdlog::info("Process successfully detached from shell.");

  // wait for cleanup
  // https://man7.org/tlpi/code/online/dist/inotify/demo_inotify.c.html
  // https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html#un-populated-notification
  spdlog::info("Waiting for cgroup cleanup");
  constexpr auto event_size = sizeof(inotify_event);
  constexpr auto buf_size = 1024 * (event_size + 16);
  std::vector<char> buf(buf_size);
  auto fd = inotify_init();
  if (fd < 0) {
    spdlog::critical("Unable to initialize inotify.");
    exit(EXIT_FAILURE);
  }
  if (inotify_add_watch(fd, events_file.c_str(), IN_MODIFY) < 0) {
    spdlog::critical("Unable to watch {}", events_file);
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
      spdlog::info("Cgroup {} is now empty, will remove it.", name);
      fs::remove(fs::path(dir));
      spdlog::info("Done removing {}. Exit.", dir);
      exit(EXIT_SUCCESS);
    }
    spdlog::info("Cgroup {} is not empty, continue waiting...", name);
    if (read(fd, buf.data(), buf_size) <= 0) {
      spdlog::critical("Failed to get event.");
      exit(EXIT_FAILURE);
    }
  }
}