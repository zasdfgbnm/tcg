#pragma once

#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class handler {
  uint8_t num_arg_;
  using f0_t = void (*)();
  using f1_t = void (*)(const std::string &);
  using f2_t = void (*)(const std::string &, const std::string &);
  using f3_t = void (*)(const std::string &, const std::string &,
                        const std::string &);
  union {
    f0_t f0;
    f1_t f1;
    f2_t f2;
    f3_t f3;
  };

public:
  handler(f0_t f0) : num_arg_(0), f0(f0) {}
  handler(f1_t f1) : num_arg_(1), f1(f1) {}
  handler(f2_t f2) : num_arg_(2), f2(f2) {}
  handler(f3_t f3) : num_arg_(3), f3(f3) {}
  uint8_t num_arg() const { return num_arg_; }
  void operator()(const char *args[]) const;
};

struct Argument {
  std::string name;
};

inline Argument operator""_var(const char *name) { return {name}; }

class Command;

struct Handler {
  std::vector<Argument> arguments;
  virtual void operator()(
      const std::unordered_map<std::string, std::string> &args) const = 0;
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
  std::vector<handler> handlers;
  std::vector<const Handler *> new_handlers;
  bool sandbox = true;

  Command(const std::string &name, const std::vector<std::string> &alias,
          const std::string &short_description,
          const std::string &long_description,
          const std::vector<handler> &handlers, bool sandbox = true);

  virtual bool defined() const { return true; }

  static const Command *get(const std::string &name);

  static const std::map<std::string, const Command *> &all() {
    return registry;
  }

  void operator()(const char *args[]) const;
};
