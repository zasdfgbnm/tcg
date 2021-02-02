#include <string>

#include <fmt/os.h>
#include <spdlog/spdlog.h>

#include "command.hpp"
#include "utils.hpp"

namespace set {

Command command(/*name =*/"set",
                /*alias =*/{},
                /*short_description =*/"TODO: Add doc",
                /*additional_note =*/R"body(TODO: Add doc)body");

std::vector<Argument> args_ = {"cgroup_name"_var, "key"_var, "value"_var};
DEFINE_HANDLER(args_, "TODO: add doc", {
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
