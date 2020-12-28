#include <string>

#include <fmt/os.h>
#include <spdlog/spdlog.h>

#include "utils.hpp"

void set(std::string name, std::string key, std::string value) {
  auto logger = spdlog::get("set");
  logger->info("Setting cgroup {}'s {} to {}...", name, key, value);
  auto d = name_dir(name, true) + "/" + key;
  logger->debug("Printing {} to {}...", value, d);
  auto out = fmt::output_file(d);
  out.print(value);
  out.close();
  logger->debug("Done printting.");
}
