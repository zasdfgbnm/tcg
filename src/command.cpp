#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <stdexcept>

#include "command.hpp"
#include "utils.hpp"

void handler::operator()(const char *args[]) const {
  assert(args[num_arg_] == nullptr);
  switch (num_arg_) {
  case 0:
    f0();
    return;
  case 1:
    f1(args[0]);
    return;
  case 2:
    f2(args[0], args[1]);
    return;
  case 3:
    f3(args[0], args[1], args[2]);
    return;
  default:
    invalid_argument();
  }
}

Handler::Handler(Command &command, const std::vector<Argument> &arguments)
    : arguments(arguments) {
  command.new_handlers.push_back(this);
}

class HandlerExecutor {
  bool compiled_ = false;
  std::unordered_map<int64_t, Handler *> handlers;
  std::unordered_map<int64_t, int64_t> arg_next;
  std::unordered_map<int64_t, int64_t> opt_next;
  std::unordered_map<int64_t, std::string> names;

  class State {
    int64_t id;
    std::unordered_map<std::string, std::string> args;
    const std::unordered_map<int64_t, Handler *> &handlers;
    const std::unordered_map<int64_t, int64_t> &arg_next;
    const std::unordered_map<int64_t, int64_t> &opt_next;
    const std::unordered_map<int64_t, std::string> &names;

    static constexpr auto get = [](auto map, auto id) {
      auto i = map.find(id);
      if (i == map.end()) {
        invalid_argument();
      }
      return i->second;
    };

  public:
    State(const std::unordered_map<int64_t, Handler *> &handlers,
          const std::unordered_map<int64_t, int64_t> &arg_next,
          const std::unordered_map<int64_t, int64_t> &opt_next,
          const std::unordered_map<int64_t, std::string> &names)
        : id(0), handlers(handlers), arg_next(arg_next), opt_next(opt_next),
          names(names) {}
    void feed(std::string text) {
      if (boost::starts_with(text, "-")) { // is option
        id = get(opt_next, id);
      } else { // is argument
        args[get(names, id)] = text;
        id = get(arg_next, id);
      }
    }
    void finalize() const {
      auto handler = get(handlers, id);
      (*handler)(args);
    }
  };

public:
  HandlerExecutor() = default;
  void compile(const std::vector<Handler *> &handlers);
  bool compiled() const { return compiled_; }
  State start() const { return {handlers, arg_next, opt_next, names}; }
};

void HandlerExecutor::compile(const std::vector<Handler *> &handlers) {
  assert(!compiled_);
  compiled_ = true;
  auto narg = [](auto h) { return h->arguments.size(); };
  auto max_length = narg(
      *std::max_element(handlers.begin(), handlers.end(),
                        [&](auto a, auto b) { return narg(a) > narg(b); }));
  std::vector<Handler *> handlers_by_narg(max_length, nullptr);
  for (auto h : handlers) {
    assert(handlers_by_narg[narg(h)] == nullptr);
    handlers_by_narg[narg(h)] = h;
  }
}

std::map<std::string, const Command *> Command::registry;

Command::Command(const std::string &name, const std::vector<std::string> &alias,
                 const std::string &short_description,
                 const std::string &long_description,
                 const std::vector<handler> &handlers, bool sandbox)
    : executor(std::make_shared<HandlerExecutor>()), name(name), alias(alias),
      short_description(short_description), long_description(long_description),
      handlers(handlers), sandbox(sandbox) {
  if (registry.find(name) != registry.end()) {
    throw std::runtime_error(
        std::string("Conflicting name. Please report a bug at: ") + url);
  }
  registry[name] = this;
  for (auto &a : alias) {
    if (registry.find(a) != registry.end()) {
      throw std::runtime_error(
          std::string("Conflicting alias. Please report a bug at: ") + url);
    }
    registry[a] = this;
  }
}

class UndefinedCommand final : public Command {
public:
  UndefinedCommand() : Command({}, {}, {}, {}, {}) {}
  bool defined() const override { return false; }
} undefined_command;

const Command *Command::get(const std::string &name) {
  auto i = registry.find(name);
  if (i == registry.end()) {
    return &undefined_command;
  }
  return registry[name];
}

void Command::operator()(const char *args[]) const {
  if (new_handlers.size() == 0) {
    assert(handers.size() > 0);
    uint8_t num_arg = 0;
    while (args[num_arg] != nullptr)
      num_arg++;
    for (auto &h : handlers) {
      if (h.num_arg() == num_arg) {
        h(args);
        return;
      }
    }
    invalid_argument();
  } else {
    assert(handers.size() == 0);
    if (executor->compiled()) {
      executor->compile(new_handlers);
    }
    auto state = executor->start();
    auto arg = args;
    while (*arg != nullptr) {
      state.feed(*(arg++));
    }
    state.finalize();
  }
}
