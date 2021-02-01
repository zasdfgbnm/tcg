#include <string>

#include <fmt/os.h>
#include <spdlog/spdlog.h>

#include "utils.hpp"

static RegisterCommand _({.name = "set",
                          .alias = {},
                          .short_description = "TODO",
                          .long_description = R"body(TODO)body"});

void set(const std::string &name, const std::string &key,
         const std::string &value) {
  auto logger = spdlog::get("set");
  logger->info("Setting cgroup {}'s {} to {}...", name, key, value);
  auto d = name_dir(name, true) + "/" + key;
  logger->debug("Printing {} to {}...", value, d);
  auto out = fmt::output_file(d);
  out.print(value);
  out.close();
  logger->debug("Done printting.");
}
