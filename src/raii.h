#pragma once
#include <string>
#include <unordered_map>
#include "settings.h"

namespace autocgrouper {

class CGroup {
  int64_t pid;
  std::string name;
};

class User {
  Settings &settings;
  std::string name;
  std::unordered_map<int64_t, CGroup> cgroups;

 public:
  User(Settings &settings, std::string name);
  User(const User &) = delete;
  User(User &&) = default;
  ~User();
  std::string setCGroup(int64_t pid, std::string);
};

class AutoCGrouper {
  Settings &settings;
  std::unordered_map<std::string, User> users;
  static AutoCGrouper *instance;
  AutoCGrouper(Settings &settings);
  ~AutoCGrouper();

 public:
  static AutoCGrouper *getInstance(Settings &settings);
  User &operator[](std::string username);
};

}  // namespace autocgrouper