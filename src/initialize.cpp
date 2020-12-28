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

void enter_chroot_jail() {
  auto logger = spdlog::get("initialize");
  auto d = root_dir() + "/";
  logger->info("Entering chroot jail at {}.", d);
  if (chdir("/proc") < 0) {
    logger->critical("Unable to chdir to /proc.", d, std::strerror(errno));
    exit(EXIT_FAILURE);
  }
  if (chroot(d.c_str()) < 0) {
    logger->critical("Unable to chroot to {}: {}.", d, std::strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void initialize() {
  setup_loggers();
  set_log_level();
  enter_chroot_jail();
}
