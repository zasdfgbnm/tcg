#include <cstdlib>
#include <iostream>
#include <string>

#include <boost/filesystem.hpp>
#include <fmt/color.h>
#include <fmt/os.h>

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
    fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
               "Name can only contain letters, digits, or underscore.\n");
    exit(EXIT_FAILURE);
  }
}

void create(std::string name_) {
  std::string name;
  if (name_ == "") {
    name = new_name();
  } else {
    validate_name(name_);
    name = name_;
  }
  std::cout << name << std::endl;

  // create new cgroup
  auto dir = name_dir(name, false);
  std::cout << "About to create " << dir << std::endl;
  fs::create_directory(dir);
  std::cout << "created" << std::endl;
  auto procs_file = dir + "/cgroup.procs";
  auto events_file = dir + "/cgroup.events";

  // add parent process to the new cgroup
  auto out = fmt::output_file(procs_file);
  auto ppid = getppid();
  out.print("{:d}", ppid);
  out.close();

  // detach from shell
  // See: http://netzmafia.de/skripten/unix/linux-daemon-howto.html
  auto pid = fork();
  if (pid < 0) {
    fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
               "Can not detach from shell.\n");
    exit(EXIT_FAILURE);
  } else if (pid > 0) {
    exit(EXIT_SUCCESS);
  }
  umask(0);
  if (setsid() < 0) {
    exit(EXIT_FAILURE);
  }
  if (chdir("/") < 0) {
    exit(EXIT_FAILURE);
  }
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  // wait for cleanup
  // https://man7.org/tlpi/code/online/dist/inotify/demo_inotify.c.html
  // https://www.kernel.org/doc/html/latest/admin-guide/cgroup-v2.html#un-populated-notification
  constexpr auto event_size = sizeof(inotify_event);
  constexpr auto buf_size = 1024 * (event_size + 16);
  std::vector<char> buf(buf_size);
  auto fd = inotify_init();
  if (fd < 0) {
    exit(EXIT_FAILURE);
  }
  if (inotify_add_watch(fd, events_file.c_str(), IN_MODIFY) < 0) {
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
      fs::remove(fs::path(dir));
      exit(EXIT_SUCCESS);
    }
    if (read(fd, buf.data(), buf_size) <= 0) {
      exit(EXIT_FAILURE);
    }
  }
}