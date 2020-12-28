#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>

#include "utils.hpp"

namespace fs = boost::filesystem;

const char *root_dir = "/sys/fs/cgroup/terminals";

std::string user_dir() {
  auto uid = getuid();
  return fmt::format("{}/{}", root_dir, uid);
}

void set_log_level() {
  auto logger = spdlog::get("initialize");
  const char *l = std::getenv("TCG_LOG_LEVEL");
  if (l == nullptr) {
    spdlog::set_level(spdlog::level::err);
    return;
  }
  std::string level(l);
  boost::algorithm::to_lower(level);
  if (level == "critical") {
    spdlog::set_level(spdlog::level::critical);
  } else if (level == "err") {
    spdlog::set_level(spdlog::level::err);
  } else if (level == "warn") {
    spdlog::set_level(spdlog::level::warn);
  } else if (level == "info") {
    spdlog::set_level(spdlog::level::info);
  } else if (level == "debug") {
    spdlog::set_level(spdlog::level::debug);
  } else {
    logger->error("Unknown log level.");
    exit(EXIT_FAILURE);
  }
}

void setup_loggers() {
  auto sink = std::make_shared<spdlog::sinks::stderr_color_sink_mt>();
  spdlog::register_logger(std::make_shared<spdlog::logger>("main", sink));
  spdlog::register_logger(std::make_shared<spdlog::logger>("initialize", sink));
  spdlog::register_logger(std::make_shared<spdlog::logger>("create", sink));
  spdlog::register_logger(std::make_shared<spdlog::logger>("list", sink));
  spdlog::register_logger(std::make_shared<spdlog::logger>("freeze", sink));
  spdlog::register_logger(std::make_shared<spdlog::logger>("utils", sink));
}

void create_root_dir() {
  auto p = fs::path(root_dir);
  if (!fs::is_directory(p)) {
    fs::create_directory(p);
  }
  p = fs::path(user_dir());
  if (!fs::is_directory(p)) {
    fs::create_directory(p);
  }
}

void enter_chroot_jail() {
  auto logger = spdlog::get("initialize");
  logger->info("Chdir to /proc so we have access to procs information.");
  if (chdir("/proc") < 0) {
    logger->critical("Unable to chdir to /proc: {}.", std::strerror(errno));
    exit(EXIT_FAILURE);
  }
  auto ud = user_dir();
  logger->info("Entering chroot jail at {}.", ud);
  if (chroot(ud.c_str()) < 0) {
    logger->critical("Unable to chroot to {}: {}.", ud.c_str(), std::strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void initialize() {
  setup_loggers();
  set_log_level();
  create_root_dir();
  enter_chroot_jail();
}
