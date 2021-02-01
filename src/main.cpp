#include <string>

#include "utils.hpp"

void invalid_argument();

void check_arg(bool condition) {
  if (condition) {
    return;
  }
  invalid_argument();
}

void initialize_logger();
void enter_sandbox();
void help();
void help(const std::string &command);
void list();
void self();
void create(const std::string &name_);
void freeze(const std::string &name);
void unfreeze(const std::string &name);
void set(const std::string &name, const std::string &key,
         const std::string &value);
void show(const std::string &name, const std::string &key);

int main(int argc, const char *argv[]) {
  initialize_logger();

  std::string command;
  if (argc == 1) {
    command = "help";
  } else {
    check_arg(argc >= 2);
    command = argv[1];
  }

  auto cmd = Command::get(command);

  if (cmd.sandbox) {
    enter_sandbox();
  }

  if (cmd.name == "help") {
    if (argc <= 2) {
      help();
    } else {
      check_arg(argc == 3);
      help(argv[2]);
    }
  } else if (cmd.name == "list") {
    check_arg(argc == 2);
    list();
  } else if (cmd.name == "self") {
    self();
  } else if (cmd.name == "create") {
    check_arg(argc == 2 || argc == 3);
    if (argc == 2) {
      create("");
    } else {
      create(argv[2]);
    }
  } else if (cmd.name == "freeze") {
    check_arg(argc == 3);
    freeze(argv[2]);
  } else if (cmd.name == "unfreeze") {
    check_arg(argc == 3);
    unfreeze(argv[2]);
  } else if (cmd.name == "set") {
    check_arg(argc == 5);
    set(argv[2], argv[3], argv[4]);
  } else if (cmd.name == "show") {
    check_arg(argc == 4);
    show(argv[2], argv[3]);
  } else {
    invalid_argument();
  }
}
