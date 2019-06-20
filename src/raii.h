#pragma once
#include <string>
#include <unordered_map>
#include "settings.h"

namespace autocgrouper {

class CGroup {
  int64_t pid;
  std::string name;
  User &user;

 public:
  CGroup(User &user, int64_t pid, const std::string & name);
  CGroup(const CGroup &) = delete;
  CGroup(CGroup &&) = default;
  ~CGroup();
  void rename(const std::string &name);
  std::string path(const std::string & cgroup);
};

class User {
  AutoCGrouper &autocgrouper;
  std::string name;
  std::unordered_map<int64_t, CGroup> cgroups;

 public:
  User(AutoCGrouper &autocgrouper, const std::string & name);
  User(const User &) = delete;
  User(User &&);
  ~User();
  void setCGroup(int64_t pid, const std::string & name);
  std::string path(const std::string & cgroup);
};

class AutoCGrouper {
  Settings &settings;
  std::unordered_map<std::string, User> users;
  static AutoCGrouper *instance;
  AutoCGrouper(Settings &settings);
  ~AutoCGrouper();

 public:
  static AutoCGrouper *getInstance(Settings &settings);
  static void terminate();
  User &operator[](const std::string & username);
  std::string path(const std::string & cgroup);
};

}  // namespace autocgrouper