#include "raii.h"

namespace autocgrouper {

AutoCGrouper::AutoCGrouper(Settings &settings) : settings(settings) {}
AutoCGrouper::~AutoCGrouper() {}
AutoCGrouper *AutoCGrouper::getInstance(Settings &settings) {
    if (nullptr == instance) {
        instance = new AutoCGrouper(settings);
    }
    return instance;
}

User::User(Settings &settings, std::string name) : settings(settings), name(name) {}
User::~User() {}

User &AutoCGrouper::operator[](std::string username) {
    if(users.count(username) == 0) {
        users[username] = User(settings, username);
    }
    return users[username];
}

}  // namespace autocgrouper