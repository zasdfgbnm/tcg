#include <fstream>
#include <string>

#include <fmt/os.h>
#include <spdlog/spdlog.h>

#include "command.hpp"
#include "utils.hpp"

std::unordered_set<std::string> suggest_existing_cgroups(std::string prefix);

namespace show {

Command command(/*name =*/"show",
                /*alias =*/{},
                /*short_description =*/"show the value of specific key",
                /*additional_note =*/"");

std::vector<std::shared_ptr<const Argument>> args_ = {
    "cgroup_name"_var->suggester(suggest_existing_cgroups), "key"_var};
DEFINE_HANDLER(args_, "show the value of the key of the specified cgroup", {
  auto logger = spdlog::get("show");
  std::string name = args.at("cgroup_name");
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
