#include <string>

#include <fmt/os.h>

#include "utils.hpp"

void freeze(std::string name) {
  auto dir = name_dir(name, true);
  auto freeze_file = dir + "/cgroup.freeze";
  auto out = fmt::output_file(freeze_file);
  out.print("1");
  out.close();
}

void unfreeze(std::string name) {
  auto dir = name_dir(name, true);
  auto freeze_file = dir + "/cgroup.freeze";
  auto out = fmt::output_file(freeze_file);
  out.print("0");
  out.close();
}
