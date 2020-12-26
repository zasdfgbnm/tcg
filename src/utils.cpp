#include "utils.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

std::string root_dir() {
  auto logger = spdlog::get("utils");
  logger->debug("Getting root directory...");
  auto uid = getuid();
  logger->debug("User id is {},", uid);
  auto d = fmt::format("/sys/fs/cgroup/user.slice/user-{0}.slice/"
                       "user@{0}.service/terminals.slice",
                       uid);
  logger->debug("so the root directory is {}.", d);
  if (!fs::is_directory(d)) {
    logger->critical("Slice is not properly set up. "
      "Please refer to https://example.com on how to setup slice.");
  }
  return d;
}

std::string name_dir(std::string name, std::optional<bool> assert_existence) {
  auto logger = spdlog::get("utils");
  logger->debug("Getting directory for {}...", name);
  auto dir = root_dir() + "/" + name;
  logger->debug("The directory should be {}.", dir);
  if (assert_existence.has_value()) {
    bool v = assert_existence.value();
    logger->debug("Check that the directory{} exist.", (v ? "": " does not"));
    if (v && !fs::is_directory(dir)) {
      logger->error("Cgroup does not exist.");
      exit(EXIT_FAILURE);
    }
    if (!v && fs::is_directory(dir)) {
      logger->error("Cgroup name already used.");
      exit(EXIT_FAILURE);
    }
    logger->debug("Directory existence check pass.");
  } else {
    logger->debug("Do not check directory existence.");
  }
  return dir;
}

std::vector<std::string> used_names() {
  auto logger = spdlog::get("utils");
  logger->debug("List all used names.");
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
  auto logger = spdlog::get("utils");
  auto d = root_dir() + "/" + name;
  logger->debug("Check if name {}, which correspond to directory {} is already used.", name, d);
  fs::path p(d);
  return fs::is_directory(p);
}

std::string new_name() {
  auto logger = spdlog::get("utils");
  logger->debug("Getting a new builtin name...");
  const static std::string names[] = {
      "newton",     "einstein", "schrodinger", "feynman", "fermi",
      "noether",    "dirac",    "kepler",      "ampere",  "pauli",
      "heisenberg", "hilbert",  "poincare",    "maxwell", "boltzmann",
      "gibbs",      "hawking",  "lorentz"};
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
