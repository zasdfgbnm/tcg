#pragma once

#include <string>
#include <vector>
#include <optional>

std::string root_dir();
std::string name_dir(std::string name, std::optional<bool> assert_existence=std::nullopt);
std::vector<std::string> used_names();
bool is_used(std::string name);
std::string new_name();
