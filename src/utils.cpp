#include "utils.hpp"
#include <boost/filesystem.hpp>
#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <unistd.h>

namespace fs = boost::filesystem;

extern bool is_chroot_jail;

bool stdout_is_tty() { return isatty(fileno(stdout)); }

fmt::text_style maybe_style(fmt::text_style style) {
  if (stdout_is_tty()) {
    return style;
  }
  return {};
}

std::string app_dir() {
  std::string root = std::string(cgroup_root);
  if (root.size() == 0) {
    return "";
  }
  return root + "terminals/";
}

std::string user_dir() {
  std::string root = app_dir();
  if (root.size() == 0) {
    return "";
  }
  if (is_chroot_jail) {
    return "/";
  }
  auto uid = getuid();
  return fmt::format("{}{}/", app_dir(), uid);
}

std::string name_dir(const std::string &name,
                     std::optional<bool> assert_existence) {
  auto logger = spdlog::get("utils");
  logger->debug("Getting directory for {}...", name);
  std::string usr = user_dir();
  if (usr.size() == 0) {
    logger->info("No cgroup mount found.");
    return "";
  }
  auto dir = usr + name;
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

const fmt::text_style error_format =
    maybe_style(fg(fmt::color::red) | fmt::emphasis::bold);
const fmt::text_style code_format = maybe_style(fmt::emphasis::underline);

void invalid_argument() {
  fmt::print(error_format, "Invalid arguments.\n");
  fmt::print("Run ");
  fmt::print(code_format, "tcg help");
  fmt::print(" for more information.\n");
  exit(EXIT_FAILURE);
}
