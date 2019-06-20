#pragma once

#include <string>
#include <vector>

namespace linuxapi {

void mkdir(const std::string & dirname);
void rmdir(const std::string & dirname);
void mvdir(const std::string & oldname, const std::string & newname);
void chown(const std::string & path, const std::string & username);
void write(const std::string & filename, const std::string & text);
std::vector<std::string> list_files(const std::string & dirname);

}
