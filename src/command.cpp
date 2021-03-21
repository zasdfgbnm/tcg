#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/assert.hpp>
#include <stdexcept>
#include <utility>
#include <queue>

#include "command.hpp"
#include "utils.hpp"

const char *LL1_ERROR = "The language is not LL(1).";

class InvalidHandler : public Handler {
public:
  InvalidHandler() : Handler(Handler::do_not_register{}) {}
  void operator()(const arg_map_t &args) const override { invalid_argument(); }
} invalid_handler;

Handler::Handler(Command &command,
                 const std::vector<std::shared_ptr<const Argument>> &arguments,
                 const std::string &description)
    : arguments(arguments), description(description) {
  command.handlers.push_back(this);
}

struct NextInfo {
  const Handler *handler = &invalid_handler;
  std::unordered_map<std::string, int64_t> keywords;
  std::string variable = "";
  int64_t variable_next = -1;
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
    if (next.keywords.contains(text)) {
      id = next.keywords.at(text);
    } else if (next.variable.size() > 0) {
      args[next.variable] = text;
      id = next.variable_next;
    } else {
      invalid_argument();
    }
  }
  void finalize() const { (*next_info().handler)(args); }
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
  int64_t id = 0;
  struct HandlerWrapper {
    const Handler *handler;
    HandlerWrapper(const Handler *handler): handler(handler) {}
    bool operator<(const HandlerWrapper &rhs) const {
      return handler->arg_size() < rhs.handler->arg_size();
    }
  };
  struct Branch {
    std::priority_queue<HandlerWrapper> handlers;
    int64_t cursor;
  };
  std::queue<Branch> branches;
  branches.emplace();
  for (auto h : handlers) {
    branches.front().handlers.emplace(h);
  }
  while (branches.size() > 0) {
    Branch &branch = branches.front();
    branches.pop();
    auto &handlers = branch.handlers;
    NextInfo info;
    while (true) {
      auto top = handlers.top();
      if (top.handler->arg_size() == branch.cursor) {
        handlers.pop();
        info.handler = top.handler;
      }
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
