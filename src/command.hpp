#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct Argument {
  std::string name;
};

inline Argument operator""_var(const char *name, size_t size) {
  return {std::string(name, size)};
}

class Command;

using arg_map_t = std::unordered_map<std::string, std::string>;

struct Handler {
  std::vector<Argument> arguments;
  virtual void operator()(const arg_map_t &args) const = 0;
  Handler(Command &, const std::vector<Argument> &);
};

class HandlerExecutor;

class Command {
  static std::map<std::string, const Command *> registry;
  std::shared_ptr<HandlerExecutor> executor;

public:
  std::string name;
  std::vector<std::string> alias;
  std::string short_description;
  std::string long_description;
  std::vector<const Handler *> handlers;
  bool sandbox = true;

  Command(const std::string &name, const std::vector<std::string> &alias,
          const std::string &short_description,
          const std::string &long_description, bool sandbox = true);

  virtual bool defined() const { return true; }

  static const Command *get(const std::string &name);

  static const std::map<std::string, const Command *> &all() {
    return registry;
  }

  void operator()(const char *args[]) const;
};
