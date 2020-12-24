#include <string>

void invalid_argument();

void check_arg(bool condition) {
  if (condition) {
    return;
  }
  invalid_argument();
}

void help();
void list();
void create(std::string name_);
void freeze(std::string name);
void unfreeze(std::string name);

int main(int argc, const char *argv[]) {
  check_arg(argc >= 2);

  std::string command = argv[1];

  if (command == "help" || command == "h") {
    help();
  }
  else if(command == "list" || command == "ls") {
    check_arg(argc == 2);
    list();
  }
  else if(command == "create" || command == "c") {
    check_arg(argc == 2 || argc == 3);
    if (argc == 2) {
      create("");
    } else {
      create(argv[2]);
    }
  }
  else if(command == "freeze" || command == "f") {
    check_arg(argc == 3);
    freeze(argv[2]);
  }
  else if(command == "unfreeze" || command == "uf") {
    check_arg(argc == 3);
    unfreeze(argv[2]);
  }
  else if(command == "limit" || command == "l") {
    check_arg(argc == 2);
  }
  else {
    invalid_argument();
  }
}
