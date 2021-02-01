#include <cstdlib>
#include <unordered_map>

#include <fmt/color.h>
#include <fmt/core.h>

#include "utils.hpp"

static RegisterHelpInfo info("name", {
  .description = "Display help information."
});

const auto name_format = fg(fmt::color::cornflower_blue);
const auto error_format = fg(fmt::color::red) | fmt::emphasis::bold;
const auto code_format = fmt::emphasis::underline;
const auto title_format = fmt::emphasis::bold;

void help() {
  fmt::print(title_format, "Usage:\n");
  fmt::print("tcg <command> [<args>]\n\n");
  fmt::print(title_format, "To get help for command:\n");
  fmt::print("tcg help <command>\n\n");
  fmt::print(title_format, "Available commands:\n");
}

void help(std::string command) {
  fmt::print(error_format, "Not supported yet\n");
}

void invalid_argument() {
  fmt::print(error_format, "Invalid arguments.\n");
  fmt::print("Run ");
  fmt::print(code_format, "tcg help");
  fmt::print(" for more information.\n");
  exit(EXIT_FAILURE);
}
