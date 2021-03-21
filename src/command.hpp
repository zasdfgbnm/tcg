#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

struct Argument {
  std::string name;
  Argument(const std::string &name) : name(name) {}
  virtual ~Argument() {}
};

struct Variable : public Argument {
  using Argument::Argument;
};

inline std::shared_ptr<const Variable> operator""_var(const char *name,
                                                      size_t size) {
  return std::make_shared<const Variable>(std::string(name, size));
}

class Keyword : public Argument {
  std::unordered_set<std::string> alias_;

public:
  Keyword(const std::string &name, const std::unordered_set<std::string> &alias)
      : Argument(name), alias_(alias) {}

  template <typename... args_t>
  std::shared_ptr<const Keyword> alias(args_t... args) const {
    std::shared_ptr<Keyword> ret = std::make_shared<Keyword>(name, alias_);
    auto new_alias = std::unordered_set<std::string>{args...};
    ret->alias_.merge(new_alias);
    return ret;
  }
  bool has_alias(std::string a) const { return alias_.contains(a); }
};

inline std::shared_ptr<const Keyword> operator""_kwd(const char *name,
                                                     size_t size) {
  return std::make_shared<const Keyword>(std::string(name, size),
                                         std::unordered_set<std::string>{});
}

class Command;

using arg_map_t = std::unordered_map<std::string, std::string>;

struct Handler {
  std::vector<std::shared_ptr<const Argument>> arguments;
  std::string description;
  virtual void operator()(const arg_map_t &args) const = 0;
  Handler(Command &, const std::vector<std::shared_ptr<const Argument>> &,
          const std::string &);

  class do_not_register {};

  Handler(do_not_register) {}
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
    operator()(const arg_map_t &args) const override code                      \
  } _MAKE_UNIQUE(handler)(command)

#define DEFINE_HANDLER(variables, description, code)                           \
  _DEFINE_HANDLER(_MAKE_UNIQUE(Handler), variables, description, code)
