#include "lib/resource.hpp"

#include "lib/testing.hpp"

namespace volc {

TEST_CASE("Application") {
  Application application{"test-app", 0};

  SECTION("ShouldPass") { REQUIRE(true); }
}

}  // namespace volc
