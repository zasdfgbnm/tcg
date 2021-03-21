#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/assert.hpp>
#include <stdexcept>
#include <utility>

#include "command.hpp"
#include "utils.hpp"

const char *LL1_ERROR = "The language is not LL(1).";

class InvalidHandler : public Handler {
public:
  InvalidHandler(): Handler(Handler::do_not_register{}) {}
  void operator()(const arg_map_t &args) const override {
    invalid_argument();
  }
} invalid_handler;

Handler::Handler(Command &command, const std::vector<std::shared_ptr<const Argument>> &arguments,
                 const std::string &description)
    : arguments(arguments), description(description) {
  command.handlers.push_back(this);
}

struct NextInfo {
  const Handler *handler = &invalid_handler;
  std::vector<std::pair<std::shared_ptr<const Argument>, int64_t>> arguments;
};

class StateMachine {
  int64_t id;
  std::unordered_map<std::string, std::string> args;
  const std::unordered_map<int64_t, NextInfo> &next_;

  const NextInfo &next_info() const {
    const auto &i = next_.find(id);
    if (i == next_.end()) {
      invalid_argument();
    }
    return i->second;
  };

public:
  StateMachine(const std::unordered_map<int64_t, NextInfo> &next)
      : id(0), next_(next) {}
  void feed(std::string text) {
    const NextInfo &next = next_info();
    for (auto &arg_id : next.arguments) {
      auto &arg = arg_id.first;
      auto next_id = arg_id.second;
      if (typeid(*arg) == typeid(Variable)) {
        // when there are multiple variables, these variables
        // must have the same name, and there can not be keyword
        // at the same position
        BOOST_ASSERT_MSG(next.arguments.size() == 1, LL1_ERROR);
        args[arg->name] = text;
        id = next_id;
        break;
      } else {
        BOOST_ASSERT_MSG(typeid(*arg) == typeid(Keyword), "Unknow argument type");
        std::shared_ptr<const Keyword> keyword = std::dynamic_pointer_cast<const Keyword>(arg);
        if (text == keyword->name || keyword->has_alias(text)) {
          id = next_id;
          break;
        }
      }
    }
  }
  void finalize() const {
    (*next_info().handler)(args);
  }
};

class HandlerExecutor {
  bool compiled_ = false;
  std::unordered_map<int64_t, NextInfo> next;

public:
  HandlerExecutor() = default;
  void compile(const std::vector<const Handler *> &handlers);
  bool compiled() const { return compiled_; }
  StateMachine start() const { return {next}; }
};

void HandlerExecutor::compile(const std::vector<const Handler *> &handlers) {
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
    NextInfo next_info;
    if (handlers_by_narg[i] != nullptr) {
      next_info.handler = handlers_by_narg[i];
    }
    if (i > 0) {
      std::string name;
      for (int64_t j = i; j <= max_length; j++) {
        auto h = handlers_by_narg[j];
        if (h == nullptr) {
          continue;
        }
        if (name.size() == 0) {
          name = h->arguments[i - 1]->name;
          next_info.arguments.emplace_back(h->arguments[i - 1], i);
        } else {
          BOOST_ASSERT_MSG(name == h->arguments[i - 1]->name, LL1_ERROR);
        }
      }
    }
    next[i] = next_info;
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
