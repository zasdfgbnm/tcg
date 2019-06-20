#include "linux.h"

namespace linuxapi {

void mkdir(const std::string& dirname) {}
void rmdir(const std::string& dirname) {}
void mvdir(const std::string& oldname, const std::string& newname) {}
void chown(const std::string& path, const std::string& username) {}
void write(const std::string& filename, const std::string& text) {}
bool exists(const std::string& filename) { return false; }
void block_until_modified(const std::string& filename) { return; }

}  // namespace linuxapi