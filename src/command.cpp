#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/assert.hpp>
#include <stdexcept>

#include "command.hpp"
#include "utils.hpp"

Handler::Handler(Command &command, const std::vector<std::shared_ptr<Argument>> &arguments,
                 const std::string &description)
    : arguments(arguments), description(description) {
  command.handlers.push_back(this);
}

class HandlerExecutor {
  bool compiled_ = false;
  std::unordered_map<int64_t, const Handler *> state_handlers;
  std::unordered_map<int64_t, int64_t> arg_next;
  std::unordered_map<int64_t, int64_t> opt_next;
  std::unordered_map<int64_t, std::string> names;

  class State {
    int64_t id;
    std::unordered_map<std::string, std::string> args;
    const std::unordered_map<int64_t, const Handler *> &state_handlers;
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
    State(const std::unordered_map<int64_t, const Handler *> &state_handlers,
          const std::unordered_map<int64_t, int64_t> &arg_next,
          const std::unordered_map<int64_t, int64_t> &opt_next,
          const std::unordered_map<int64_t, std::string> &names)
        : id(0), state_handlers(state_handlers), arg_next(arg_next),
          opt_next(opt_next), names(names) {}
    void feed(std::string text) {
      if (boost::starts_with(text, "-")) { // is option
        id = get(opt_next, id);
      } else { // is argument
        args[get(names, id)] = text;
        id = get(arg_next, id);
      }
    }
    void finalize() const {
      auto handler = get(state_handlers, id);
      (*handler)(args);
    }
  };

public:
  HandlerExecutor() = default;
  void compile(const std::vector<const Handler *> &handlers);
  bool compiled() const { return compiled_; }
  State start() const { return {state_handlers, arg_next, opt_next, names}; }
};

void HandlerExecutor::compile(const std::vector<const Handler *> &handlers) {
  const char *LL1_ERROR = "The language is not LL(1).";
  assert(!compiled_);
  compiled_ = true;
  auto narg = [](auto h) { return h->arguments.size(); };
  auto max_length = narg(
      *std::max_element(handlers.begin(), handlers.end(),
                        [&](auto a, auto b) { return narg(a) < narg(b); }));
  std::vector<const Handler *> handlers_by_narg(max_length + 1, nullptr);
  for (auto h : handlers) {
    auto n = narg(h);
    BOOST_ASSERT_MSG(handlers_by_narg[n] == nullptr, LL1_ERROR);
    handlers_by_narg[n] = h;
  }
  for (int64_t i = 0; i <= max_length; i++) {
    if (i > 0) {
      std::string name;
      for (int64_t j = i; j <= max_length; j++) {
        auto h = handlers_by_narg[j];
        if (h == nullptr) {
          continue;
        }
        if (name.size() == 0) {
          name = h->arguments[i - 1]->name;
        } else {
          BOOST_ASSERT_MSG(name == h->arguments[i - 1]->name, LL1_ERROR);
        }
      }
      names[i - 1] = name;
    }
    if (handlers_by_narg[i] != nullptr) {
      state_handlers[i] = handlers_by_narg[i];
    }
    if (i > 0) {
      arg_next[i - 1] = i;
    }
  }
}

std::map<std::string, const Command *> Command::registry;

Command::Command(const std::string &name, const std::vector<std::string> &alias,
                 const std::string &short_description,
                 const std::string &additional_note, bool sandbox)
    : executor(std::make_shared<HandlerExecutor>()), name(name), alias(alias),
      short_description(short_description), additional_note(additional_note),
      sandbox(sandbox) {
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
  if (!executor->compiled()) {
    executor->compile(handlers);
  }
  auto state = executor->start();
  auto arg = args;
  while (*arg != nullptr) {
    state.feed(*(arg++));
  }
  state.finalize();
}
