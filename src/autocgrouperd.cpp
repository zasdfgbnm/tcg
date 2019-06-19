#include <iostream>
#include "settings.h"

int main(int argc, char *argv[], char *envp[]) {
  Settings settings = try_load_settings("/etc/autocgrouper.yml");
}