#pragma once

#include <fmt/color.h>
#include <optional>
#include <string>

extern const char *cgroup_root;
constexpr const char *url = "https://github.com/zasdfgbnm/tcg";

bool stdout_is_tty();
fmt::text_style maybe_style(fmt::text_style style);

void invalid_argument();

std::string app_dir();
std::string user_dir();
std::string name_dir(const std::string &name,
                     std::optional<bool> assert_existence = std::nullopt);

inline bool startswith(std::string str, std::string prefix) {
  return str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix;
}
