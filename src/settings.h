#pragma once
#include <string>
#include <vector>

namespace autocgrouper {

class Settings {
 public:
  // fallback settings
  Settings()
      : cgroupfs_path("/sys/fs/cgroup/unified"),
        use_nickname(false),
        check_ms(1000){};

  // load settings from file, not implemented yet
  Settings(const std::string& filename) = delete;

  std::string cgroupfs_path;
  bool use_nickname;
  std::vector<std::string> nicknames;
  int64_t check_ms;
};

Settings try_load_settings(const std::string& filename);

}  // namespace autocgrouper
