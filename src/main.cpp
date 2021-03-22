#include <string>

#include "command.hpp"

void initialize_logger();
void enter_sandbox();

int main(int argc, const char *argv[]) {
  initialize_logger();

  if (argc < 2) {
    const char *args[] = {nullptr};
    Command::get("help")->execute(args);
    return 0;
  }

  auto cmd = Command::get(argv[1]);

  if (cmd->sandbox) {
    enter_sandbox();
  }

  cmd->execute(argv + 2);
}
