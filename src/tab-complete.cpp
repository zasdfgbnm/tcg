#include <cstdlib>

#include <boost/assert.hpp>
#include <fmt/core.h>

#include "command.hpp"
#include "utils.hpp"

std::unordered_set<std::string> suggest_commands(std::string prefix) {
  std::unordered_set<std::string> result;
  for (auto &i : Command::all()) {
    if (!i.second->defined() || i.first != i.second->name) {
      continue;
    }
    if (startswith(i.first, prefix)) {
      result.insert(i.first);
    } else {
      for (auto &j : i.second->alias) {
        if (startswith(j, prefix)) {
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

std::vector<std::shared_ptr<const Argument>> args1 = {
    "command"_var->suggester(suggest_commands)};
DEFINE_HANDLER(args1, "complete arguments", {
  std::string partial_command = args.at("command");
  auto suggestions = suggest_commands(partial_command);
  for (auto &i : suggestions) {
    fmt::print("{}\n", i);
  }
});

std::vector<std::shared_ptr<const Argument>> args2 = {
    "command"_var->suggester(suggest_commands), "args"_varargs};
DEFINE_HANDLER(args2, "complete arguments", {
  auto cmd = Command::get(args.at("command"));
  auto suggestions = cmd->suggest(varargs);
  for (auto &i : suggestions) {
    fmt::print("{}\n", i);
  }
});

} // namespace tab_complete
