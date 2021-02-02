#include <string>

#include <fmt/os.h>
#include <spdlog/spdlog.h>

#include "command.hpp"
#include "utils.hpp"

namespace freeze {

Command command(/*name =*/"freeze",
                /*alias =*/{"f"},
                /*short_description =*/"TODO: Add doc",
                /*additional_note =*/R"body(TODO: Add doc)body");

DEFINE_HANDLER({"cgroup_name"_var}, "freeze the specified cgroup", {
  auto logger = spdlog::get("freeze");
  std::string name = args.at("cgroup_name");
  logger->info("Will freeze {}.", name);
  auto dir = name_dir(name, true);
  auto freeze_file = dir + "/cgroup.freeze";
  logger->info("Writing 1 to {}...", freeze_file);
  auto out = fmt::output_file(freeze_file);
  out.print("1");
  out.close();
  logger->info("Done freezing {}.", name);
});

} // namespace freeze

namespace unfreeze {

Command command(/*name =*/"unfreeze",
                /*alias =*/{"uf"},
                /*short_description =*/"TODO: Add doc",
                /*additional_note =*/R"body(TODO: Add doc)body");

DEFINE_HANDLER({"cgroup_name"_var}, "unfreeze the specified cgroup", {
  auto logger = spdlog::get("freeze");
  std::string name = args.at("cgroup_name");
  logger->info("Will unfreeze {}.", name);
  auto dir = name_dir(name, true);
  auto freeze_file = dir + "/cgroup.freeze";
  logger->info("Writing 0 to {}...", freeze_file);
  auto out = fmt::output_file(freeze_file);
  out.print("0");
  out.close();
  logger->info("Done unfreezing {}.", name);
});

} // namespace unfreeze
