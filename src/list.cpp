#include <iostream>
#include <string>
#include <vector>

#include "utils.hpp"

std::ostream &operator<<(std::ostream &os, const std::vector<std::string> &v) {
  bool first = true;
  for (auto i : v) {
    if (!first) {
      os << ", ";
    }
    os << i;
    first = false;
  }
  return os;
}

void list() { std::cout << used_names() << std::endl; }
