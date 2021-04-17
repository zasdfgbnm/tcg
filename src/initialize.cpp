#include <cerrno>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <unordered_set>

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/string_file.hpp>
#include <fmt/os.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "config.h"
#include "utils.hpp"

#ifdef USE_SECCOMP
#include <seccomp.h>
#endif

const char *cgroup_root = "";

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
  spdlog::register_logger(std::make_shared<spdlog::logger>("set", sink));
  spdlog::register_logger(std::make_shared<spdlog::logger>("self", sink));
  spdlog::register_logger(std::make_shared<spdlog::logger>("show", sink));
}

bool file_contains(std::shared_ptr<spdlog::logger> logger,
                   const std::string &file, const std::string &controller) {
  logger->debug("Testing if {} already contains {}.", file, controller);
  std::ifstream in(file);
  std::string s;
  while (std::getline(in, s)) {
    if (s.find(controller) != std::string::npos) {
      return true;
    }
  }
  return false;
}

void set_cgroup_root() {
  auto logger = spdlog::get("initialize");
  if (boost::filesystem::exists("/sys/fs/cgroup/cgroup.procs")) {
    // cgroup v2
    logger->debug("Found cgroup v2");
    cgroup_root = "/sys/fs/cgroup/";
  } else if (boost::filesystem::exists("/sys/fs/cgroup/unified/cgroup.procs")) {
    // cgroup hybrid
    logger->debug("Found cgroup hybrid");
    cgroup_root = "/sys/fs/cgroup/unified/";
  }
}

void warn_hybrid(std::shared_ptr<spdlog::logger> logger) {
  logger->warn(
      "You are using cgroup v2 in hybrid mode. In this mode, many controllers "
      "will be unaccessable to cgroup v2 because it is already used by cgroup "
      "v1. tcg relies on cgroup v2 and can not access these v1 controllers. To "
      "get better experience, it is recommended to use pure cgroup v2. See: "
      "https://wiki.archlinux.org/index.php/Cgroups#Switching_to_cgroups_v2");
}

void enable_controllers(std::shared_ptr<spdlog::logger> logger,
                        const std::string &dir) {
  std::unordered_set<std::string> controllers;
  // read controllers from cgroup.controllers
  std::string controller;
  std::string filename = dir + "cgroup.controllers";
  logger->debug("Reading: {}", filename);
  std::ifstream in(dir + "cgroup.controllers");
  while (in >> controller) {
    logger->debug("Get controller: {}", controller);
    controllers.insert(controller);
  }
  if (cgroup_root == "/sys/fs/cgroup/unified/" &&
      (!controllers.contains("cpu") || !controllers.contains("memory") ||
       !controllers.contains("io"))) {
    warn_hybrid(logger);
  }

  // enable controller for subtree
  for (std::string c : controllers) {
    auto subtree_control = dir + "cgroup.subtree_control";
    if (file_contains(logger, subtree_control, c)) {
      logger->debug(
          "The controller {} of {} is already enabled, has nothing to do.", c,
          dir);
      continue;
    }
    logger->debug("Set subtree_control of {} for controller {}.", dir, c);
    auto text = "+" + c;
    logger->debug("Printting \"{}\" to {}.", text, dir);
    auto out = fmt::output_file(subtree_control);
    out.print(text);
    out.close();
    logger->debug("Done printting.");
  }
}

void create_root_dir(std::shared_ptr<spdlog::logger> logger) {
  logger->info("Initialize root directory.");
  enable_controllers(logger, cgroup_root);
  auto p = fs::path(app_dir());
  logger->debug("Check if {} exist.", app_dir());
  if (!fs::is_directory(p)) {
    logger->debug("{} does not exist, create it.", app_dir());
    fs::create_directory(p);
  }
  enable_controllers(logger, app_dir());
  auto ud = user_dir();
  logger->debug("Check if {} exist.", ud);
  p = fs::path(ud);
  if (!fs::is_directory(p)) {
    logger->debug("{} does not exist, create it.", ud);
    fs::create_directory(p);
  }
  enable_controllers(logger, ud);
}

bool is_chroot_jail = false;

void enter_chroot_jail(std::shared_ptr<spdlog::logger> logger) {
// disable chroot jail for CODE_COVERAGE to allow writing to profile
#ifndef CODE_COVERAGE
  logger->info("Entering chroot jail...");
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
  logger->info("Chroot jail entered.");
  is_chroot_jail = true;
#endif
}

void initialize_logger() {
  setup_loggers();
  set_log_level();
}

void check_cgroup_mount(std::shared_ptr<spdlog::logger> logger) {
  auto p = std::string(cgroup_root);
  if (p.size() == 0) {
    logger->critical("Cgroup v2 not mounted");
    exit(EXIT_FAILURE);
  }
}

void check_euid(std::shared_ptr<spdlog::logger> logger) {
  if (geteuid() == 0) {
    return;
  }
  logger->critical(
      "The tcg executable need to be owned by root and has suid permission.");
  exit(EXIT_FAILURE);
}

void setup_seccomp() {
#ifdef USE_SECCOMP
  auto logger = spdlog::get("initialize");
  logger->info("Setting up seccomp...");
  scmp_filter_ctx ctx;
  ctx = seccomp_init(SCMP_ACT_KILL_PROCESS);

  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(open), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(openat), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(read), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(write), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(lseek), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(close), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(chdir), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(chroot), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(mkdir), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rmdir), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(umask), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(newfstatat), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(stat), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(statx), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(getdents64), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(ioctl), 0);

  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(inotify_init), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(inotify_add_watch), 0);

  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit_group), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(exit), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(fork), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(clone), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(brk), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(futex), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(sigprocmask), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(rt_sigprocmask), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(set_robust_list), 0);

  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(getuid), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(geteuid), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(gettid), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(getpid), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(getppid), 0);
  seccomp_rule_add(ctx, SCMP_ACT_ALLOW, SCMP_SYS(setsid), 0);
  seccomp_load(ctx);
#endif
}

void enter_sandbox() {
  auto logger = spdlog::get("initialize");
  logger->info("Entering sandbox...");
  check_cgroup_mount(logger);
  check_euid(logger);
  create_root_dir(logger);
  enter_chroot_jail(logger);
  logger->info("Sandbox entered successfully.");
}

void initialize() {
  initialize_logger();
  set_cgroup_root();
  setup_seccomp();
}
