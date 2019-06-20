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

User &AutoCGrouper::operator[](std::string username) {
  if (users.count(username) == 0) {
    users.emplace(username, User(settings, username));
  }
  return users.at(username);
}

}  // namespace autocgrouper