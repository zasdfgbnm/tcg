#include <string>

#include <fmt/os.h>
#include <spdlog/spdlog.h>

#include "utils.hpp"

void freeze(const std::string &name) {
  auto logger = spdlog::get("freeze");
  logger->info("Will freeze {}.", name);
  auto dir = name_dir(name, true);
  auto freeze_file = dir + "/cgroup.freeze";
  logger->info("Writing 1 to {}...", freeze_file);
  auto out = fmt::output_file(freeze_file);
  out.print("1");
  out.close();
  logger->info("Done freezing {}.", name);
}

void unfreeze(const std::string &name) {
  auto logger = spdlog::get("freeze");
  logger->info("Will unfreeze {}.", name);
  auto dir = name_dir(name, true);
  auto freeze_file = dir + "/cgroup.freeze";
  logger->info("Writing 0 to {}...", freeze_file);
  auto out = fmt::output_file(freeze_file);
  out.print("0");
  out.close();
  logger->info("Done unfreezing {}.", name);
}

static Command command_f(/*name =*/"freeze",
                         /*alias =*/{"f"},
                         /*short_description =*/"TODO: Add doc",
                         /*long_description =*/R"body(TODO: Add doc)body",
                         /*handlers =*/{freeze});

static Command command_uf(/*name =*/"unfreeze",
                          /*alias =*/{"uf"},
                          /*short_description =*/"TODO: Add doc",
                          /*long_description =*/R"body(TODO: Add doc)body",
                          /*handlers =*/{unfreeze});
