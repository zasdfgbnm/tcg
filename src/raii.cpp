#include "raii.h"
#include "linux.h"

namespace autocgrouper {

AutoCGrouper::AutoCGrouper(Settings &settings) : settings(settings) {
  linuxapi::mkdir(path());
}

AutoCGrouper::~AutoCGrouper() { linuxapi::rmdir(path()); }

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

std::string AutoCGrouper::path() {
  return settings.cgroupfs_path + "/autocgrouper";
}

User &AutoCGrouper::operator[](const std::string &username) {
  if (users.count(username) == 0) {
    users.emplace(username, User(*this, username));
  }
  return users.at(username);
}

User::User(AutoCGrouper &autocgrouper, const std::string &name)
    : autocgrouper(autocgrouper), name(name) {
  linuxapi::mkdir(path());
}
User::~User() { linuxapi::rmdir(path()); }

User::User(User &&other) : User(other.autocgrouper, other.name) {
  std::swap(cgroups, other.cgroups);
}

void User::setCGroup(int64_t pid, const std::string &name) {
  if (cgroups.count(pid) == 0) {
    cgroups.emplace(pid, CGroup(*this, pid, name));
  } else {
    cgroups.at(pid).rename(name);
  }
}

std::string User::path() { return autocgrouper.path() + "/" + name; }

CGroup::CGroup(User &user, int64_t pid, const std::string &name)
    : user(user), pid(pid), name(name) {
  std::string dir = path();
  linuxapi::mkdir(dir);
  for (std::string filename : linuxapi::list_files(dir)) {
    if (filename != "cgroup.procs") {
      linuxapi::chown(filename, user.name);  // TODO: Is it better to use DELEGATION?
    }
  }
  linuxapi::append(dir + "/cgroup.procs", std::to_string(pid));
  // TODO: monitor populated event
}

CGroup::~CGroup() { linuxapi::rmdir(path()); }

void CGroup::rename(const std::string &name) {
  linuxapi::mvdir(path(), path(name));
  this->name = name;
}

std::string CGroup::path() { return path(name); }

std::string CGroup::path(const std::string &name) {
  return user.path() + "/" + name;
}

}  // namespace autocgrouper