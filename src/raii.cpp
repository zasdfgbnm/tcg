#include "raii.h"

namespace autocgrouper {

User::User(Settings &settings) : settings(settings) {}
AutoCGrouper::AutoCGrouper(Settings &settings) : settings(settings) {}

User::~User() {}
AutoCGrouper::~AutoCGrouper() {}

}  // namespace autocgrouper