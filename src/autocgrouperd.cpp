#include "raii.h"
#include "settings.h"

using namespace autocgrouper;

int main(int argc, char *argv[], char *envp[]) {
  Settings settings = try_load_settings("/etc/autocgrouper.yml");
  AutoCGrouper autocgrouper(settings);
}