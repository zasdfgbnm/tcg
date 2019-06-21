#include "raii.h"
#include "linux.h"
#include <mutex>
#include <shared_mutex>
#include <thread>

namespace autocgrouper {

const char *nonprivileged_files[] = {
    ""  // TODO: fill this out
};

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
    std::unique_lock lock(mutex);
    cgroups.emplace(pid, CGroup(*this, pid, name));
    std::thread cleanup_thread([&, pid](){
      std::unique_lock lock(mutex);
      linuxapi::block_until_modified(cgroups.at(pid).path() + "/cgroup.events");
      cgroups.erase(pid);
    });
    cleanup_thread.detach();
  } else {
    std::shared_lock lock(mutex);
    cgroups.at(pid).rename(name);
  }
}

std::string User::path() { return autocgrouper.path() + "/" + name; }

CGroup::CGroup(User &user, int64_t pid, const std::string &name)
    : user(user), pid(pid), name(name) {
  std::string dir = path();
  linuxapi::mkdir(dir);
  for (std::string filename : nonprivileged_files) {
    filename = dir + filename;
    if (linuxapi::exists(filename)) {
      linuxapi::chown(filename, user.name);
    }
  }
  linuxapi::write(dir + "/cgroup.procs", std::to_string(pid));
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