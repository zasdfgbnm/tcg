#include <fstream>
#include <string>

#include <fmt/os.h>
#include <spdlog/spdlog.h>

#include "command.hpp"
#include "utils.hpp"

static Command command(/*name =*/"show",
                       /*alias =*/{},
                       /*short_description =*/"TODO: Add doc",
                       /*long_description =*/R"body(TODO: Add doc)body",
                       /*handlers =*/{});

static struct ShowHandler final : public Handler {
  ShowHandler(Command &command) : Handler(command, {"name"_var, "key"_var}) {}
  void operator()(
      const std::unordered_map<std::string, std::string> &args) const override {
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
  }
} help_handler(command);
