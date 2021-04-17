#include <fmt/color.h>
#include <fmt/core.h>

#include "command.hpp"
#include "config.h"
#include "utils.hpp"

namespace version {

const fmt::text_style name_format =
    maybe_style(fg(fmt::color::cornflower_blue));

Command command(/*name =*/"version",
                /*alias =*/{"v"},
                /*short_description =*/"display version information",
                /*additional_note =*/"",
                /*sandbox =*/false // disable sandbox to allow users to read
                                   // docs on systems without cgroup v2
);

void print_item(const char *property, const char *value) {
  fmt::print(name_format, "{}:", property);
  fmt::print(" {}\n", value);
}

void show_verison() {
#ifdef VERSION
  print_item("Version", VERSION);
#else
  print_item("Version", "unknown");
#endif

#ifdef GIT_COMMIT
  print_item("Git commit", GIT_COMMIT);
#else
  print_item("Git commit", "unknown");
#endif

#ifdef BUILD_DATE
  print_item("Build date", BUILD_DATE);
#else
  print_item("Build date", "unknown");
#endif

#ifdef CMAKE_VERSION
  print_item("CMake version", CMAKE_VERSION);
#else
  print_item("CMake version", "unknown");
#endif
}

DEFINE_HANDLER({}, "show version information", { show_verison(); });

}
