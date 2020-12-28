#include <string>
#include <unistd.h>
#include <iostream>

#include <spdlog/spdlog.h>
#include <fmt/core.h>

#include "utils.h"

void self() {
  auto pid = getpid();
  auto cg = fmt::format("/proc/{}/cgroup", pid);
  auto ud = user_dir();
  std::string line;
  ifstream in(cg);
  while (std::getline(in, line)) {
    if (line.find(ud) != std::string::npos) {
      fmt::print("found")!
    }
  }
}