#include "lib/resource.hpp"

#include "lib/testing.hpp"

namespace volc::lib {

TEST_CASE("Application") {
  Application application{"test-app", 0, Application::DebugSeverity::VERBOSE};

  SECTION("ShouldPass") { REQUIRE(true); }
}

}  // namespace volc::lib
