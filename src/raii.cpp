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

User::User(Settings &settings, std::string name)
    : settings(settings), name(name) {
  // mkdir
}
User::~User() {
  // rmdir
}

User::User(User &&other) : User(other.settings, other.name) {
  std::swap(cgroups, other.cgroups);
}

void User::setCGroup(int64_t pid, std::string name) {
  if (cgroups.count(pid) == 0) {
    cgroups.emplace(pid, CGroup(pid, name, this->name));
  } else {
    cgroups.at(pid).rename(name);
  }
}

User &AutoCGrouper::operator[](std::string username) {
  if (users.count(username) == 0) {
    users.emplace(username, User(settings, username));
  }
  return users.at(username);
}

CGroup::CGroup(int64_t pid, std::string name, std::string username)
    : pid(pid), name(name), username(username) {
  // mkdir
}

CGroup::~CGroup() {
  // rmdir
}

void CGroup::rename(std::string name) {
  // mv dir
}

}  // namespace autocgrouper