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
  std::string description;
  virtual void operator()(const arg_map_t &args) const = 0;
  Handler(Command &, const std::vector<Argument> &, const std::string &);
};

class HandlerExecutor;

class Command {
  static std::map<std::string, const Command *> registry;
  std::shared_ptr<HandlerExecutor> executor;

public:
  std::string name;
  std::vector<std::string> alias;
  std::string short_description;
  std::string additional_note;
  std::vector<const Handler *> handlers;
  bool sandbox = true;

  Command(const std::string &name, const std::vector<std::string> &alias,
          const std::string &short_description,
          const std::string &additional_note, bool sandbox = true);

  virtual bool defined() const { return true; }

  static const Command *get(const std::string &name);

  static const std::map<std::string, const Command *> &all() {
    return registry;
  }

  void operator()(const char *args[]) const;
};

// boiler-plate to create unique name:
// https://stackoverflow.com/a/2419720
#define _CONCATENATE_DETAIL(x, y) x##y
#define _CONCATENATE(x, y) _CONCATENATE_DETAIL(x, y)
#define _MAKE_UNIQUE(x) _CONCATENATE(x, __COUNTER__)

#define _DEFINE_HANDLER(name, variables, description, code)                    \
  struct name final : public Handler{                                          \
    name(Command & command) : Handler(command, variables, description){} void  \
    operator()(const std::unordered_map<std::string, std::string> &args)       \
        const override code                                                    \
  } _MAKE_UNIQUE(handler)(command)

#define DEFINE_HANDLER(variables, description, code)                           \
  _DEFINE_HANDLER(_MAKE_UNIQUE(Handler), variables, description, code)
