#include <cstdlib>

#include <fmt/color.h>
#include <fmt/core.h>

const auto name_format = fg(fmt::color::cornflower_blue);
const auto error_format = fg(fmt::color::red) | fmt::emphasis::bold;
const auto code_format = fmt::emphasis::underline;
const auto title_format = fmt::emphasis::bold;

const char *help_description = "display help information";

void help() {
  fmt::print(title_format, "Usage:\n");
  fmt::print("tcg command [args...]\n\n");
  fmt::print(title_format, "Available commands:\n");
  fmt::print(name_format, "help: ");
  fmt::print(help_description);
}

void invalid_argument() {
  fmt::print(error_format, "Invalid arguments.\n");
  fmt::print("Run ");
  fmt::print(code_format, "tcg help");
  fmt::print(" for more information.\n");
  exit(EXIT_FAILURE);
  ;
}
