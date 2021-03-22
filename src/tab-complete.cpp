#include <cstdlib>

#include <fmt/core.h>
#include <spdlog/spdlog.h>
#include <boost/assert.hpp>

#include "command.hpp"

namespace tab_complete {

Command command("tab-complete",
                /*alias =*/{},
                /*short_description =*/"suggest next token",
                /*additional_note =*/"",
                /*sandbox =*/false // disable sandbox to allow users to read
                                   // docs on systems without cgroup v2
);

DEFINE_HANDLER({"args"_varargs}, "complete arguments", {
  auto logger = spdlog::get("tab-complete");
  BOOST_ASSERT_MSG(varargs.size() == 1, "Not implemented yet.");
  std::string partial_command = varargs[0];
  if (partial_command.back() != '\t') {
    logger->error("Must end with <tab>");
    exit(EXIT_FAILURE);
  }
  partial_command = partial_command.substr(0, partial_command.size() - 1);
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

} // namespace tab_complete
