#include <cstdlib>

#include <spdlog/spdlog.h>
#include <fmt/core.h>

#include "command.hpp"

namespace tab_complete {

Command command("tab-complete",
                /*alias =*/{},
                /*short_description =*/"suggest next token",
                /*additional_note =*/"");

DEFINE_HANDLER({"partial_command"_var}, "complete partial command", {
  auto logger = spdlog::get("tab-complete");
  std::string partial_command = args.at("partial_command");
  if (partial_command.back() != '\t') {
    logger->error("Must end with <tab>");
    exit(EXIT_FAILURE);
  }
  partial_command = partial_command.substr(0, partial_command.size() - 1);
  fmt::print(partial_command);
});

} // namespace tab_complete
