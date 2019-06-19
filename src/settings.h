#pragma once
#include <string>
#include <vector>

class Settings {
 public:
  // fallback settings
  Settings()
      : cgroupfs_path("/sys/fs/cgroup"),
        cgroups({"cpu,cpuacct"}),
        use_nickname(false){};

  // load settings from file, not implemented yet
  Settings(std::string filename) = delete;

  std::string cgroupfs_path;
  std::vector<std::string> cgroups;
  bool use_nickname;
  std::vector<std::string> nicknames;
};

Settings try_load_settings(std::string filename);