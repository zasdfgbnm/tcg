#include "utils.hpp"

#include <fmt/color.h>
#include <fmt/format.h>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

std::string root_dir() {
  auto uid = getuid();
  auto d = fmt::format("/sys/fs/cgroup/user.slice/user-{0}.slice/"
                       "user@{0}.service/terminals.slice",
                       uid);
  if (!fs::is_directory(d)) {
    fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
               "Slice is not properly set up.\n"
               "Please refer to https://example.com on how to setup slice.");
  }
  return d;
}

std::string name_dir(std::string name, std::optional<bool> assert_existence) {
  auto dir = root_dir() + "/" + name;
  if (assert_existence.has_value()) {
    bool v = assert_existence.value();
    if (v && !fs::is_directory(dir)) {
      fmt::print(fg(fmt::color::red) | fmt::emphasis::bold, "Invalid name.");
      exit(EXIT_FAILURE);
    }
    if (!v && fs::is_directory(dir)) {
      fmt::print(fg(fmt::color::red) | fmt::emphasis::bold,
                 "Name already used.");
      exit(EXIT_FAILURE);
    }
  }
  return dir;
}

std::vector<std::string> used_names() {
  std::vector<std::string> names;
  fs::path p(root_dir());
  fs::recursive_directory_iterator end;
  for (fs::recursive_directory_iterator i(p); i != end; ++i) {
    if (fs::is_directory(*i)) {
      names.push_back(i->path().filename().string());
    }
  }
  return names;
}

bool is_used(std::string name) {
  fs::path p(root_dir() + "/" + name);
  return fs::is_directory(p);
}

std::string new_name() {
  const static std::string names[] = {
      "newton",     "einstein", "schrodinger", "feynman", "fermi",
      "noether",    "dirac",    "kepler",      "ampere",  "pauli",
      "heisenberg", "hilbert",  "poincare",    "maxwell", "boltzmann",
      "gibbs",      "hawking",  "lorentz"};
  for (auto n : names) {
    if (!is_used(n)) {
      return n;
    }
  }
  fmt::print(fg(fmt::color::red) | fmt::emphasis::bold, "Run out of names.\n");
  exit(EXIT_FAILURE);
}
