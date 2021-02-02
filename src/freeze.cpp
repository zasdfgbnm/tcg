#include <string>

#include <fmt/os.h>
#include <spdlog/spdlog.h>

#include "command.hpp"
#include "utils.hpp"

static Command command_f(/*name =*/"freeze",
                         /*alias =*/{"f"},
                         /*short_description =*/"TODO: Add doc",
                         /*long_description =*/R"body(TODO: Add doc)body");

static struct FreezeHandler final : public Handler {
  FreezeHandler(Command &command) : Handler(command, {"name"_var}) {}
  void operator()(
      const std::unordered_map<std::string, std::string> &args) const override {
    auto logger = spdlog::get("freeze");
    std::string name = args.at("name");
    logger->info("Will freeze {}.", name);
    auto dir = name_dir(name, true);
    auto freeze_file = dir + "/cgroup.freeze";
    logger->info("Writing 1 to {}...", freeze_file);
    auto out = fmt::output_file(freeze_file);
    out.print("1");
    out.close();
    logger->info("Done freezing {}.", name);
  }
} freeze_handler(command_f);

static Command command_uf(/*name =*/"unfreeze",
                          /*alias =*/{"uf"},
                          /*short_description =*/"TODO: Add doc",
                          /*long_description =*/R"body(TODO: Add doc)body");

static struct UnfreezeHandler final : public Handler {
  UnfreezeHandler(Command &command) : Handler(command, {"name"_var}) {}
  void operator()(
      const std::unordered_map<std::string, std::string> &args) const override {
    auto logger = spdlog::get("freeze");
    std::string name = args.at("name");
    logger->info("Will unfreeze {}.", name);
    auto dir = name_dir(name, true);
    auto freeze_file = dir + "/cgroup.freeze";
    logger->info("Writing 0 to {}...", freeze_file);
    auto out = fmt::output_file(freeze_file);
    out.print("0");
    out.close();
    logger->info("Done unfreezing {}.", name);
  }
} unfreeze_handler(command_uf);
