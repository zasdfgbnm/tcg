#include <cstdlib>
#include <unordered_map>

#include <fmt/color.h>
#include <fmt/core.h>

#include "command.hpp"
#include "utils.hpp"

namespace help {

const fmt::text_style name_format =
    maybe_style(fg(fmt::color::cornflower_blue));
const fmt::text_style title_format = maybe_style(fmt::emphasis::bold);
const fmt::text_style error_format =
    maybe_style(fg(fmt::color::red) | fmt::emphasis::bold);

void usage() {
  // title
  fmt::print(title_format, "Usage:\n");
  fmt::print("tcg <command> [<args>]\n\n");

  // help
  fmt::print(title_format, "To get help for command:\n");
  fmt::print("tcg help <command>\n\n");

  // commands
  fmt::print(title_format, "Available Commands:\n");
  for (auto &i : Command::all()) {
    if (!i.second->defined() || i.first != i.second->name) {
      continue;
    }
    fmt::print(name_format, i.first + ": ");
    fmt::print(i.second->short_description);
    fmt::print("\n");
  }
  fmt::print("\n");

  // alias
  fmt::print(title_format, "Aliases of Commands:\n");
  for (auto &i : Command::all()) {
    if (!i.second->defined() || i.first != i.second->name) {
      continue;
    }
    if (i.second->alias.size() > 0) {
      fmt::print(name_format, i.first + ": ");
      bool first = true;
      for (auto &a : i.second->alias) {
        if (!first) {
          fmt::print(", ");
        }
        fmt::print(a);
        first = false;
      }
      fmt::print("\n");
    }
  }
  fmt::print("\n");

  // more info
  fmt::print(title_format, "For more information, go to:\n");
  fmt::print(url);
  fmt::print("\n");
}

Command command(/*name =*/"help",
                /*alias =*/{"h"},
                /*short_description =*/"display help information",
                /*long_description =*/R"body(
There are two ways of using help:
  - tcg help
  - tcg help <command>
The former shows the help information for the entire tcg tool, and the latter
shows the help for a specific command.)body",
                /*sandbox =*/false // disable sandbox to allow users to read
                                   // docs on systems without cgroup v2
);

DEFINE_HANDLER({}, { usage(); });

DEFINE_HANDLER({"command"_var}, {
  auto c = Command::get(args.at("command"));
  if (!c->defined()) {
    fmt::print(error_format, "Unknown command.");
    return;
  }

  // title
  fmt::print(title_format | name_format, c->name + ": ");
  fmt::print(title_format, c->short_description);
  fmt::print("\n\n");

  // alias
  if (c->alias.size() > 0) {
    fmt::print(title_format, "Alias: ");
    bool first = true;
    for (auto &i : c->alias) {
      if (!first) {
        fmt::print(", ");
      }
      fmt::print(name_format, i);
      first = false;
    }
    fmt::print("\n");
  }

  // long description
  fmt::print(c->long_description);
});

} // namespace help