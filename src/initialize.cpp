#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <fmt/os.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

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

void enable_controllers(std::shared_ptr<spdlog::logger> logger,
                        std::string dir) {
  logger->debug("Set subtree_control of {}.", dir);
  auto subtree_control = dir + "/cgroup.subtree_control";
  auto text = "+cpu";
  logger->debug("Printting \"{}\" to {}.", text, dir);
  auto out = fmt::output_file(subtree_control);
  out.print(text);
  out.close();
  logger->debug("Done printting.");
}

void create_root_dir() {
  auto logger = spdlog::get("initialize");
  logger->info("Initialize root directory.");
  enable_controllers(logger, "/sys/fs/cgroup");
  auto p = fs::path(root_dir);
  logger->debug("Check if {} exist.", root_dir);
  if (!fs::is_directory(p)) {
    logger->debug("{} does not exist, create it.", root_dir);
    fs::create_directory(p);
    enable_controllers(logger, root_dir);
  }
  auto ud = user_dir();
  logger->debug("Check if {} exist.", ud);
  p = fs::path(ud);
  if (!fs::is_directory(p)) {
    logger->debug("{} does not exist, create it.", ud);
    fs::create_directory(p);
    enable_controllers(logger, ud);
  }
}

void enter_chroot_jail() {
  auto logger = spdlog::get("initialize");
  logger->info("Enter chroot jail.");
  auto ud = user_dir();
  logger->debug("Chdir to {}.", ud);
  if (chdir(ud.c_str()) < 0) {
    logger->critical("Unable to chdir to {}: {}.", ud, std::strerror(errno));
    exit(EXIT_FAILURE);
  }
  logger->debug("Entering chroot jail at {}.", ud);
  if (chroot(ud.c_str()) < 0) {
    logger->critical("Unable to chroot to {}: {}.", ud.c_str(),
                     std::strerror(errno));
    exit(EXIT_FAILURE);
  }
}

void initialize() {
  setup_loggers();
  set_log_level();
}

bool is_sandbox;

void enter_sandbox() {
  create_root_dir();
  enter_chroot_jail();
  is_sandbox = true;
}
