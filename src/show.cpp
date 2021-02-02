#include <fstream>
#include <string>

#include <fmt/os.h>
#include <spdlog/spdlog.h>

#include "command.hpp"
#include "utils.hpp"

namespace show {

Command command(/*name =*/"show",
                /*alias =*/{},
                /*short_description =*/"TODO: Add doc",
                /*long_description =*/R"body(TODO: Add doc)body");

std::vector<Argument> args_ = {"name"_var, "key"_var};
DEFINE_HANDLER(args_, {
  auto logger = spdlog::get("show");
  std::string name = args.at("name");
  std::string key = args.at("key");
  logger->info("Showing cgroup {}'s {}...", name, key);
  auto d = name_dir(name, true) + "/" + key;
  logger->debug("Reading {}...", d);
  std::ifstream in(d);
  std::string line;
  while (std::getline(in, line)) {
    fmt::print("{}\n", line);
  }
  logger->debug("Done printting.");
});

} // namespace show
