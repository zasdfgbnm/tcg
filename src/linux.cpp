#include "linux.h"

namespace linuxapi {

void mkdir(const std::string& dirname) {}
void rmdir(const std::string& dirname) {}
void mvdir(const std::string& oldname, const std::string& newname) {}
void chown(const std::string& path, const std::string& username) {}
void append(const std::string& filename, const std::string& text) {}
std::vector<std::string> list_files(const std::string& dirname) { return {}; }
int filesize(const std::string& filename) { return 0; }

}  // namespace linuxapi