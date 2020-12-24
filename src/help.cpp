#include <cstdlib>

#include <fmt/core.h>
#include <fmt/color.h>

void help() {
  fmt::print("Usage:\n");
  fmt::print("tcg command [args...]\n");
}

void invalid_argument() {
  fmt::print(fg(fmt::color::red) | fmt::emphasis::bold, "Invalid arguments.\n");
  fmt::print("Run ");
  fmt::print(fmt::emphasis::underline, "tcg help");
  fmt::print(" for more information.\n");
  exit(EXIT_FAILURE);;
}
