#include <cstdlib>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

#include <boost/algorithm/string.hpp>

void invalid_argument();

void check_arg(bool condition) {
  if (condition) {
    return;
  }
  invalid_argument();
}

void set_log_level() {
  auto logger = spdlog::get("main");
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
  spdlog::register_logger(std::make_shared<spdlog::logger>("create", sink));
  spdlog::register_logger(std::make_shared<spdlog::logger>("list", sink));
  spdlog::register_logger(std::make_shared<spdlog::logger>("freeze", sink));
  spdlog::register_logger(std::make_shared<spdlog::logger>("utils", sink));
}

void help();
void list();
void create(std::string name_);
void freeze(std::string name);
void unfreeze(std::string name);
void limit(std::string name, std::string resource, std::string value);

int main(int argc, const char *argv[]) {
  setup_loggers();
  set_log_level();
  check_arg(argc >= 2);

  std::string command = argv[1];

  if (command == "help" || command == "h") {
    help();
  } else if (command == "list" || command == "ls") {
    check_arg(argc == 2);
    list();
  } else if (command == "create" || command == "c") {
    check_arg(argc == 2 || argc == 3);
    if (argc == 2) {
      create("");
    } else {
      create(argv[2]);
    }
  } else if (command == "freeze" || command == "f") {
    check_arg(argc == 3);
    freeze(argv[2]);
  } else if (command == "unfreeze" || command == "uf") {
    check_arg(argc == 3);
    unfreeze(argv[2]);
  } else if (command == "limit" || command == "l") {
    check_arg(argc == 5);
    limit(argv[2], argv[3], argv[4]);
  } else {
    invalid_argument();
  }
}
