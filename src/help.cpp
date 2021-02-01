#include <cstdlib>
#include <unordered_map>

#include <fmt/color.h>
#include <fmt/core.h>

#include "utils.hpp"

const auto name_format = fg(fmt::color::cornflower_blue);
const auto error_format = fg(fmt::color::red) | fmt::emphasis::bold;
const auto code_format = fmt::emphasis::underline;
const auto title_format = fmt::emphasis::bold;

void help0() {
  fmt::print(title_format, "Usage:\n");
  fmt::print("tcg <command> [<args>]\n\n");
  fmt::print(title_format, "To get help for command:\n");
  fmt::print("tcg help <command>\n\n");
  fmt::print(title_format, "Available commands:\n");
  for (auto &i : Command::all()) {
    fmt::print(name_format, i.first + ": ");
    fmt::print(i.second.short_description);
    fmt::print("\n");
  }
  fmt::print("\n");
  fmt::print(title_format, "For more information, go to:\n");
  fmt::print(url);
}

void help1(const std::string &command) {
  const auto &info = Command::get(command);
  if (info.name.size() == 0) {
    fmt::print(error_format, "Unknown command.");
    return;
  }

  // title
  fmt::print(title_format | name_format, info.name + ": ");
  fmt::print(title_format, info.short_description);
  fmt::print("\n\n");

  // alias
  if (info.alias.size() > 0) {
    fmt::print(title_format, "Alias: ");
    bool first = true;
    for (auto &i : info.alias) {
      if (!first) {
        fmt::print(", ");
      }
      fmt::print(name_format, i);
      first = false;
    }
    fmt::print("\n");
  }

  // long description
  fmt::print(info.long_description);
}

void invalid_argument() {
  fmt::print(error_format, "Invalid arguments.\n");
  fmt::print("Run ");
  fmt::print(code_format, "tcg help");
  fmt::print(" for more information.\n");
  exit(EXIT_FAILURE);
}

static RegisterCommand
    _({.name = "help",
       .alias = {"h"},
       .sandbox = false, // disable sandbox to allow users to read docs on
                         // systems without cgroup v2
       .short_description = "display help information",
       .long_description = R"body(
There are two ways of using help:
  - tcg help
  - tcg help <command>
The former shows the help information for the entire tcg tool, and the latter
shows the help for a specific command.)body",
       .handlers = {help0, help1}});
