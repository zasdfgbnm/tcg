#include <string>

#include <fmt/os.h>
#include <spdlog/spdlog.h>

#include "command.hpp"
#include "utils.hpp"

std::unordered_set<std::string> suggest_existing_cgroups(std::string prefix);

namespace set {

Command command(/*name =*/"set",
                /*alias =*/{},
                /*short_description =*/"set the value of specific key",
                /*additional_note =*/"");

std::vector<std::shared_ptr<const Argument>> args_ = {"cgroup_name"_var->suggester(suggest_existing_cgroups),
                                                      "key"_var, "value"_var};
DEFINE_HANDLER(args_, "set the value of the key of the specified cgroup", {
  auto logger = spdlog::get("set");
  std::string name = args.at("cgroup_name");
  std::string key = args.at("key");
  std::string value = args.at("value");
  logger->info("Setting cgroup {}'s {} to {}...", name, key, value);
  auto d = name_dir(name, true) + "/" + key;
  logger->debug("Printing {} to {}...", value, d);
  auto out = fmt::output_file(d);
  out.print(value);
  out.close();
  logger->debug("Done printting.");
});

} // namespace set
