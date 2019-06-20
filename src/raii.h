#pragma once
#include "settings.h"
#include <unordered_map>
#include <string>

namespace autocgrouper{

class CGroup {
};

class User {
    Settings &settings;
    std::string name;
    std::unordered_map<std::string, CGroup> cgroups;
public:
    User(Settings &settings);
};

class AutoCGrouper {
    Settings &settings;
public:
    AutoCGrouper(Settings &settings);

    std::unordered_map<int64_t, User> users;
};

}