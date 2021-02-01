#include <cstdlib>

#include <fmt/color.h>
#include <fmt/core.h>

const char *help_description = "display help information";

void help() {
  fmt::print("Usage:\n");
  fmt::print("tcg command [args...]\n\n");
  fmt::print("Available commands:\n");
  fmt::print(fg(fmt::color::blue) | fmt::emphasis::bold, "help: ");
  fmt::print(help_description);
}

void invalid_argument() {
  fmt::print(fg(fmt::color::red) | fmt::emphasis::bold, "Invalid arguments.\n");
  fmt::print("Run ");
  fmt::print(fmt::emphasis::underline, "tcg help");
  fmt::print(" for more information.\n");
  exit(EXIT_FAILURE);
  ;
}
