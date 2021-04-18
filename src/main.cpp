#include <string>

#include "command.hpp"

void initialize1();
void enter_chroot();
void initialize2(const Flags &flags);

int main(int argc, const char *argv[]) {
  initialize1();

  if (argc < 2) {
    const char *args[] = {nullptr};
    Command::get("help")->execute(args);
    return 0;
  }

  auto cmd = Command::get(argv[1]);
  initialize2(cmd->flags);

  cmd->execute(argv + 2);
}
