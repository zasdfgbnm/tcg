#include "raii.h"

namespace autocgrouper {

AutoCGrouper::AutoCGrouper(Settings &settings) : settings(settings) {
  // mkdir
}

AutoCGrouper::~AutoCGrouper() {
  // rmdir
}

AutoCGrouper *AutoCGrouper::instance = nullptr;

AutoCGrouper *AutoCGrouper::getInstance(Settings &settings) {
  if (nullptr == instance) {
    instance = new AutoCGrouper(settings);
  }
  return instance;
}

void AutoCGrouper::terminate() {
  delete instance;
  instance = nullptr;
}

std::string AutoCGrouper::path(std::string cgroup) {
  return settings.cgroupfs_path + "/" + cgroup + "/autocgrouper";
}

User &AutoCGrouper::operator[](std::string username) {
  if (users.count(username) == 0) {
    users.emplace(username, User(*this, username));
  }
  return users.at(username);
}

User::User(AutoCGrouper &autocgrouper, std::string name)
    : autocgrouper(autocgrouper), name(name) {
  // mkdir
}
User::~User() {
  // rmdir
}

User::User(User &&other) : User(other.autocgrouper, other.name) {
  std::swap(cgroups, other.cgroups);
}

void User::setCGroup(int64_t pid, std::string name) {
  if (cgroups.count(pid) == 0) {
    cgroups.emplace(pid, CGroup(*this, pid, name));
  } else {
    cgroups.at(pid).rename(name);
  }
}

std::string User::path(std::string cgroup) {
  return autocgrouper.path(cgroup) + "/" + name;
}

CGroup::CGroup(User &user, int64_t pid, std::string name)
    : user(user), pid(pid), name(name) {
  // mkdir
}

CGroup::~CGroup() {
  // rmdir
}

void CGroup::rename(std::string name) {
  // mv dir
  this->name = name;
}

std::string CGroup::path(std::string cgroup) {
  return user.path(cgroup) + "/" + name;
}

}  // namespace autocgrouper