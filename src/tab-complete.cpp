#include <cstdlib>

#include <boost/assert.hpp>
#include <fmt/core.h>
#include <spdlog/spdlog.h>

#include "command.hpp"

namespace tab_complete {

Command command("tab-complete",
                /*alias =*/{},
                /*short_description =*/"suggest next token",
                /*additional_note =*/"",
                /*sandbox =*/false // disable sandbox to allow users to read
                                   // docs on systems without cgroup v2
);

DEFINE_HANDLER({"command"_var}, "complete arguments", {
  auto logger = spdlog::get("tab-complete");
  std::string partial_command = args.at("command");
  for (auto &i : Command::all()) {
    if (!i.second->defined() || i.first != i.second->name) {
      continue;
    }
    auto match = [&](std::string str) {
      return str.size() >= partial_command.size() &&
             str.substr(0, partial_command.size()) == partial_command;
    };
    if (match(i.first)) {
      fmt::print("{}\n", i.first);
    } else {
      for (auto &j : i.second->alias) {
        if (match(j)) {
          fmt::print("{}\n", j);
        }
      }
    }
  }
});

std::vector<std::shared_ptr<const Argument>> args_ = {"command"_var,
                                                      "args"_varargs};
DEFINE_HANDLER(args_, "complete arguments", {
  auto cmd = Command::get(args.at("command"));
  auto result = cmd->suggest(varargs);
  for (auto &i : result) {
    fmt::print("{}\n", i);
  }
});

} // namespace tab_complete
