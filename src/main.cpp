#include <string>

void invalid_argument();

void check_arg(bool condition) {
  if (condition) {
    return;
  }
  invalid_argument();
}

void initialize_logger();
void check_cgroup_mount();
void enter_sandbox();
void help();
void list();
void self();
void create(std::string name_);
void freeze(std::string name);
void unfreeze(std::string name);
void set(std::string name, std::string key, std::string value);
void show(std::string name, std::string key);

int main(int argc, const char *argv[]) {
  initialize_logger();
  check_arg(argc >= 2);

  std::string command = argv[1];

  if (command == "help" || command == "h") {
    // help should run without sandbox because
    // this will allow users to read docs about
    // this software on a machine without cgroup v2
    // support
    help();
    return 0;
  }

  check_cgroup_mount();

  if (command == "list" || command == "ls" || command == "l") {
    // list has to run outside sandbox because
    // it needs access to /proc filesystem
    check_arg(argc == 2);
    list();
    return 0;
  } else if (command == "self" || command == "sf") {
    // self has to run outside sandbox because
    // it needs access to /proc filesystem
    self();
    return 0;
  } else {
    enter_sandbox();
  }

  if (command == "create" || command == "c") {
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
  } else if (command == "set") {
    check_arg(argc == 5);
    set(argv[2], argv[3], argv[4]);
  } else if (command == "show") {
    check_arg(argc == 4);
    show(argv[2], argv[3]);
  } else {
    invalid_argument();
  }
}
