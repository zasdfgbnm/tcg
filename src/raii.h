#pragma once
#include <string>
#include <unordered_map>
#include "settings.h"

namespace autocgrouper {

class AutoCGrouper;
class User;

class CGroup {
  const int64_t pid;
  std::string name;

 public:
  CGroup(User &user, int64_t pid, const std::string &name);
  CGroup(const CGroup &) = delete;
  CGroup(CGroup &&) = default;
  ~CGroup();
  void rename(const std::string &name);
  std::string path(const std::string &cgroup);
  std::string path(const std::string &cgroup, const std::string &name);

  User &user;
};

class User {
  std::unordered_map<int64_t, CGroup> cgroups;

 public:
  User(AutoCGrouper &autocgrouper, const std::string &name);
  User(const User &) = delete;
  User(User &&);
  ~User();
  void setCGroup(int64_t pid, const std::string &name);
  std::string path(const std::string &cgroup);

  const std::string name;
  AutoCGrouper &autocgrouper;
};

class AutoCGrouper {
  std::unordered_map<std::string, User> users;
  static AutoCGrouper *instance;
  AutoCGrouper(Settings &settings);
  ~AutoCGrouper();

 public:
  static AutoCGrouper *getInstance(Settings &settings);
  static void terminate();
  User &operator[](const std::string &username);
  std::string path(const std::string &cgroup);
  Settings &settings;
};

}  // namespace autocgrouper