#include <string>

#include "utils.hpp"

void initialize_logger();
void enter_sandbox();
void help0();

int main(int argc, const char *argv[]) {
  initialize_logger();

  if (argc < 2) {
    help0();
    return 0;
  }

  auto cmd = Command::get(argv[1]);

  if (cmd->sandbox) {
    enter_sandbox();
  }

  cmd->call(argv + 2);
}
