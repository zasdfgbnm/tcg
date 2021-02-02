#include <string>

#include "command.hpp"

void initialize_logger();
void enter_sandbox();

namespace help {
void usage();
} // namespace help

int main(int argc, const char *argv[]) {
  initialize_logger();

  if (argc < 2) {
    help::usage();
    return 0;
  }

  auto cmd = Command::get(argv[1]);

  if (cmd->sandbox) {
    enter_sandbox();
  }

  (*cmd)(argv + 2);
}
