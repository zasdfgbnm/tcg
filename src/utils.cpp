#include "utils.hpp"

#include <fmt/color.h>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

extern bool is_sandbox;

std::string user_dir() {
  if (is_sandbox) {
    return "/";
  }
  auto uid = getuid();
  return fmt::format("{}/{}/", root_dir, uid);
}

std::string name_dir(std::string name, std::optional<bool> assert_existence) {
  auto logger = spdlog::get("utils");
  logger->debug("Getting directory for {}...", name);
  auto dir = user_dir() + name;
  logger->debug("The directory should be {}.", dir);
  if (assert_existence.has_value()) {
    bool v = assert_existence.value();
    logger->debug("Check that the directory{} exist.", (v ? "" : " does not"));
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

bool is_used(std::string name) {
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
