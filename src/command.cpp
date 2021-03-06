#include <algorithm>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/assert.hpp>
#include <deque>
#include <ranges>
#include <stdexcept>
#include <utility>

#include "command.hpp"
#include "utils.hpp"

class InvalidHandler : public Handler {
public:
  InvalidHandler() : Handler(Handler::do_not_register{}) {}
  void operator()(const arg_map_t &args,
                  const vararg_t &varargs) const override {
    invalid_argument();
  }
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
  std::unordered_map<std::string, std::string> alias;
  bool is_varargs = false;
  std::string variable = "";
  int64_t variable_next = -1;
  std::vector<Variable::suggester_t> suggesters;
};

const NextInfo invalid_next_info;

class StateMachine {
  int64_t id;
  std::unordered_map<std::string, std::string> args;
  std::vector<std::string> varargs;
  const std::unordered_map<int64_t, NextInfo> &next_;

  const NextInfo &next_info() const {
    if (next_.contains(id)) {
      return next_.at(id);
    }
    return invalid_next_info;
  };

public:
  StateMachine(const std::unordered_map<int64_t, NextInfo> &next)
      : id(0), next_(next) {}
  void feed(std::string text) {
    const NextInfo &next = next_info();
    if (next.keywords.contains(text)) {
      id = next.keywords.at(text);
    } else if (next.alias.contains(text)) {
      auto kwd = next.alias.at(text);
      id = next.keywords.at(kwd);
    } else if (next.variable.size() > 0) {
      if (next.is_varargs) {
        varargs.push_back(text);
      } else {
        args[next.variable] = text;
      }
      id = next.variable_next;
    } else {
      id = -1;
    }
  }

  void execute() const { (*next_info().handler)(args, varargs); }

  std::unordered_set<std::string> suggest(std::string prefix) {
    std::unordered_set<std::string> result;
    const NextInfo &next = next_info();
    auto match = [&](std::string str) {
      return str.size() >= prefix.size() &&
             str.substr(0, prefix.size()) == prefix;
    };
    for (auto &kv : next.keywords) {
      if (match(kv.first)) {
        result.insert(kv.first);
      }
    }
    for (auto &kv : next.alias) {
      if (match(kv.first) && !result.contains(kv.second)) {
        result.insert(kv.first);
      }
    }
    for (auto &s : next.suggesters) {
      auto new_suggestions = s(prefix);
      result.insert(new_suggestions.begin(), new_suggestions.end());
    }
    return result;
  }
};

class HandlerExecutor {
  bool compiled_ = false;
  std::unordered_map<int64_t, NextInfo> next;

public:
  void compile(const std::vector<const Handler *> &handlers);
  bool compiled() const { return compiled_; }
  StateMachine start() const { return {next}; }
};

void HandlerExecutor::compile(const std::vector<const Handler *> &handlers) {
  assert(!compiled_);
  compiled_ = true;
  struct Branch {
    std::vector<const Handler *> handlers = {};
    int64_t cursor;
    int64_t id;
    bool is_varargs;
    Branch(int64_t id, int64_t cursor, std::vector<const Handler *> handlers)
        : id(id), cursor(cursor), is_varargs(false), handlers(handlers) {}
  };

  // at the beginning, put all handlers in the same branch,
  // set cursor to 0, use id 0, and allocate a new NextInfo
  // to this id
  int64_t id = 0;
  std::deque<Branch> branches{Branch(id++, 0, handlers)};

  while (branches.size() > 0) {
    // pop the front of branches
    Branch branch = branches.front();
    branches.pop_front();
    NextInfo &next_info = next[branch.id];

    // varargs branch is handled separately, because it can
    // accept infinite number of arguments.
    if (branch.is_varargs) {
      BOOST_ASSERT_MSG(branch.handlers.size() == 1,
                       "BUG: varargs branch can only has 1 handler");
      auto handler = branch.handlers[0];
      auto &hargs = handler->arguments;
      BOOST_ASSERT_MSG(hargs.size() == branch.cursor,
                       "BUG: varargs must be the last argument");
      next_info.is_varargs = true;
      next_info.variable = hargs[branch.cursor - 1]->name;
      next_info.variable_next = branch.id;
    }

    // walk through all handlers in this branch, try
    // move cursor to the next, discover new branches
    int starting_id = id;
    for (auto h : branch.handlers) {
      auto &hargs = h->arguments;
      BOOST_ASSERT_MSG(hargs.size() >= branch.cursor,
                       "BUG: handlers shouldn't be in this branch");
      if (hargs.size() == branch.cursor) {
        // there can not be more than one handler ending
        // at the same branch, otherwise this language is not unique
        BOOST_ASSERT_MSG(next_info.handler == &invalid_handler,
                         "BUG: in one branch, only one handler can end here");
        next_info.handler = h;
      } else {
        std::shared_ptr<const Argument> harg = hargs[branch.cursor];

        // find if this handler fits an existing branch
        Branch *new_branch = nullptr;
        for (auto &b : std::ranges::views::reverse(branches)) {
          if (b.id < starting_id) {
            break;
          }
          BOOST_ASSERT_MSG(b.cursor == branch.cursor + 1,
                           "BUG: cursor for new branches not properly set.");
          BOOST_ASSERT_MSG(b.handlers.size() > 0,
                           "BUG: handlers for new branches not properly set.");
          std::shared_ptr<const Argument> barg =
              b.handlers[0]->arguments[branch.cursor];
          int compare_result = (*harg <=> *barg);
          BOOST_ASSERT_MSG(compare_result >= 0,
                           "BUG: Incompatible argument configuration.");
          if (compare_result == 0) {
            new_branch = &b;
            break;
          }
        }

        // if this handler fits to an existing branch, then add this handler to
        // this branch, otherwise, create a new branch
        if (new_branch != nullptr) {
          new_branch->handlers.push_back(h);
        } else {
          // create a new branch
          branches.emplace_back(id++, branch.cursor + 1,
                                std::vector<const Handler *>{h});
          new_branch = &branches.back();

          // update NextInfo of current branch to point to the new branch
          if (typeid(*harg) == typeid(Variable) ||
              typeid(*harg) == typeid(Varargs)) {
            bool is_varargs = (typeid(*harg) == typeid(Varargs));
            if (next_info.variable.size() == 0 &&
                next_info.variable_next == -1) {
              next_info.is_varargs = is_varargs;
              next_info.variable = harg->name;
              next_info.variable_next = new_branch->id;
            }
            new_branch->is_varargs = is_varargs;
            if (!is_varargs) {
              auto var = std::dynamic_pointer_cast<const Variable>(harg);
              if (var->suggest.has_value()) {
                next_info.suggesters.push_back(var->suggest.value());
              }
            }
            // At the same branch, different handlers can not have different
            // variables at the branching position. For example:
            //
            // legal:
            // tcg aaa, tcg aaa <bbb>, tcg aaa <bbb> <ccc>
            //
            // illegal:
            // tcg aaa <bbb>, tcg aaa <ccc>
            const char *msg = "BUG: different variables at the same position.";
            BOOST_ASSERT_MSG(next_info.is_varargs == is_varargs, msg);
            BOOST_ASSERT_MSG(next_info.variable == harg->name, msg);
            BOOST_ASSERT_MSG(next_info.variable_next == new_branch->id, msg);
          } else {
            BOOST_ASSERT_MSG(typeid(*harg) == typeid(Keyword),
                             "BUG: Unknown argument type.");
            auto check_conflict = [&](std::string k) {
              BOOST_ASSERT_MSG(!next_info.keywords.contains(k),
                               "BUG: keyword or alias conflict.");
              BOOST_ASSERT_MSG(!next_info.alias.contains(k),
                               "BUG: keyword or alias conflict.");
            };
            auto kwd = std::dynamic_pointer_cast<const Keyword>(harg);
            check_conflict(kwd->name);
            next_info.keywords[kwd->name] = new_branch->id;
            for (auto &alias : kwd->alias()) {
              check_conflict(alias);
              next_info.alias[alias] = kwd->name;
            }
          }
        }
      }
    }
  }
}

std::map<std::string, const Command *> Command::registry;

Command::Command(const std::string &name, const std::vector<std::string> &alias,
                 const std::string &short_description,
                 const std::string &additional_note, Flags flags)
    : executor(std::make_shared<HandlerExecutor>()), name(name), alias(alias),
      short_description(short_description), additional_note(additional_note),
      flags(flags) {
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

  void execute(const char *args[]) const override { invalid_argument(); }

  std::unordered_set<std::string>
  suggest(const std::vector<std::string> &args) const override {
    return {};
  }
} undefined_command;

const Command *Command::get(const std::string &name) {
  auto i = registry.find(name);
  if (i == registry.end()) {
    return &undefined_command;
  }
  return registry[name];
}

void Command::execute(const char *args[]) const {
  if (!executor->compiled()) {
    executor->compile(handlers);
  }
  auto vm = executor->start();
  auto arg = args;
  while (*arg != nullptr) {
    vm.feed(*(arg++));
  }
  vm.execute();
}

std::unordered_set<std::string>
Command::suggest(const std::vector<std::string> &args) const {
  if (!executor->compiled()) {
    executor->compile(handlers);
  }
  auto vm = executor->start();
  for (int64_t i = 0; i < args.size() - 1; i++) {
    vm.feed(args[i]);
  }
  return vm.suggest(args[args.size() - 1]);
}
