#include <cstdlib>
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
  std::string level = std::getenv("TCG_LOG_LEVEL");
  boost::algorithm::to_lower(level);
  spdlog::set_level(spdlog::level::err);
  if (level == "critical") {
    spdlog::set_level(spdlog::level::critical);
  } else if (level == "error" || level == "") {
    spdlog::set_level(spdlog::level::err);
  } else if (level == "warn") {
    spdlog::set_level(spdlog::level::warn);
  } else if (level == "info") {
    spdlog::set_level(spdlog::level::info);
  } else if (level == "debug") {
    spdlog::set_level(spdlog::level::debug);
  } else {
    spdlog::error("Unknown log level.");
  }
}

void help();
void list();
void create(std::string name_);
void freeze(std::string name);
void unfreeze(std::string name);

int main(int argc, const char *argv[]) {
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
    check_arg(argc == 2);
  } else {
    invalid_argument();
  }
}
