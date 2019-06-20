#include "raii.h"
#include "linux.h"

namespace autocgrouper {

AutoCGrouper::AutoCGrouper(Settings &settings) : settings(settings) {
  for (std::string cgroup : settings.cgroups) {
    linuxapi::mkdir(path(cgroup));
  }
}

AutoCGrouper::~AutoCGrouper() {
  for (std::string cgroup : settings.cgroups) {
    linuxapi::rmdir(path(cgroup));
  }
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

std::string AutoCGrouper::path(const std::string & cgroup) {
  return settings.cgroupfs_path + "/" + cgroup + "/autocgrouper";
}

User &AutoCGrouper::operator[](const std::string & username) {
  if (users.count(username) == 0) {
    users.emplace(username, User(*this, username));
  }
  return users.at(username);
}

User::User(AutoCGrouper &autocgrouper, const std::string & name)
    : autocgrouper(autocgrouper), name(name) {
  for (std::string cgroup : autocgrouper.settings.cgroups) {
    linuxapi::mkdir(path(cgroup));
  }
}
User::~User() {
  for (std::string cgroup : autocgrouper.settings.cgroups) {
    linuxapi::rmdir(path(cgroup));
  }
}

User::User(User &&other) : User(other.autocgrouper, other.name) {
  std::swap(cgroups, other.cgroups);
}

void User::setCGroup(int64_t pid, const std::string & name) {
  if (cgroups.count(pid) == 0) {
    cgroups.emplace(pid, CGroup(*this, pid, name));
  } else {
    cgroups.at(pid).rename(name);
  }
}

std::string User::path(const std::string & cgroup) {
  return autocgrouper.path(cgroup) + "/" + name;
}

CGroup::CGroup(User &user, int64_t pid, const std::string & name)
    : user(user), pid(pid), name(name) {
  for (std::string cgroup : user.autocgrouper.settings.cgroups) {
    linuxapi::mkdir(path(cgroup));
  }
}

CGroup::~CGroup() {
  for (std::string cgroup : user.autocgrouper.settings.cgroups) {
    linuxapi::rmdir(path(cgroup));
  }
}

void CGroup::rename(const std::string & name) {
  for (std::string cgroup : user.autocgrouper.settings.cgroups) {
    linuxapi::mvdir(path(cgroup), );
  }
  this->name = name;
}

std::string CGroup::path(const std::string & cgroup) {
  return path(cgroup, name);
}

std::string CGroup::path(const std::string & cgroup, const std::string &name) {
  return user.path(cgroup) + "/" + name;
}

}  // namespace autocgrouper