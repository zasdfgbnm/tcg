#include <string>

#include <fmt/os.h>
#include <spdlog/spdlog.h>

#include "command.hpp"
#include "utils.hpp"

static Command command(/*name =*/"set",
                       /*alias =*/{},
                       /*short_description =*/"TODO: Add doc",
                       /*long_description =*/R"body(TODO: Add doc)body");

static struct SetHandler final : public Handler {
  SetHandler(Command &command)
      : Handler(command, {"name"_var, "key"_var, "value"_var}) {}
  void operator()(
      const std::unordered_map<std::string, std::string> &args) const override {
    auto logger = spdlog::get("set");
    std::string name = args.at("name");
    std::string key = args.at("key");
    std::string value = args.at("value");
    logger->info("Setting cgroup {}'s {} to {}...", name, key, value);
    auto d = name_dir(name, true) + "/" + key;
    logger->debug("Printing {} to {}...", value, d);
    auto out = fmt::output_file(d);
    out.print(value);
    out.close();
    logger->debug("Done printting.");
  }
} handler(command);
