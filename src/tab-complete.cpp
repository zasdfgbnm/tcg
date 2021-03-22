#include <cstdlib>

#include <boost/assert.hpp>
#include <fmt/core.h>

#include "command.hpp"

std::unordered_set<std::string> suggest_commands(std::string prefix) {
  std::unordered_set<std::string> result;
  auto match = [&](std::string str) {
    return str.size() >= prefix.size() &&
           str.substr(0, prefix.size()) == prefix;
  };
  for (auto &i : Command::all()) {
    if (!i.second->defined() || i.first != i.second->name) {
      continue;
    }
    if (match(i.first)) {
      result.insert(i.first);
    } else {
      for (auto &j : i.second->alias) {
        if (match(j)) {
          result.insert(j);
        }
      }
    }
  }
  return result;
}

namespace tab_complete {

Command command("tab-complete",
                /*alias =*/{},
                /*short_description =*/"suggest next token",
                /*additional_note =*/"",
                /*sandbox =*/false // disable sandbox to allow users to read
                                   // docs on systems without cgroup v2
);

DEFINE_HANDLER({"command"_var->suggester(suggest_commands)}, "complete arguments", {
  std::string partial_command = args.at("command");
  auto suggestions = suggest_commands(partial_command);
  for (auto &i : suggestions) {
    fmt::print("{}\n", i);
  }
});

std::vector<std::shared_ptr<const Argument>> args_ = {"command"_var->suggester(suggest_commands),
                                                      "args"_varargs};
DEFINE_HANDLER(args_, "complete arguments", {
  auto cmd = Command::get(args.at("command"));
  auto suggestions = cmd->suggest(varargs);
  for (auto &i : suggestions) {
    fmt::print("{}\n", i);
  }
});

} // namespace tab_complete
