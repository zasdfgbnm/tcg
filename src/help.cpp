#include <cstdlib>
#include <unordered_map>

#include <fmt/color.h>
#include <fmt/core.h>

#include "utils.hpp"

static HelpInfo::reg ister("help", {.description = "display help information",
                                    .body = R"body(
There are two ways of using help:
  - tcg help
  - tcg help <command>
The former shows the help information for the entire tcg tool, and the latter
shows the help for a specific command.)body"});

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
  for (auto &i : HelpInfo::all()) {
    fmt::print(name_format, i.first + ": ");
    fmt::print(i.second.description);
    fmt::print("\n");
  }
}

void help(const std::string &command) {
  const auto &info = HelpInfo::get(command);
  if (info.description.size() == 0) {
    fmt::print(error_format, "Unknown command.");
    return;
  }
  fmt::print(title_format | name_format, command + ": ");
  fmt::print(title_format, info.description);
  fmt::print("\n");
  fmt::print(info.body);
}

void invalid_argument() {
  fmt::print(error_format, "Invalid arguments.\n");
  fmt::print("Run ");
  fmt::print(code_format, "tcg help");
  fmt::print(" for more information.\n");
  exit(EXIT_FAILURE);
}
