#pragma once

#include <optional>
#include <string>
#include <vector>

constexpr const char *root_dir = "/sys/fs/cgroup/terminals";

std::string user_dir();
std::string name_dir(std::string name,
                     std::optional<bool> assert_existence = std::nullopt);
bool is_used(std::string name);
std::string new_name();
