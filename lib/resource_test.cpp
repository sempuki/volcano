#include "lib/resource.hpp"

#include "lib/testing.hpp"

namespace volc {

TEST_CASE("Application") {
  Application application{"test-app", 0};
  auto instance = application.CreateInstance();

  SECTION("ShouldPass") { REQUIRE(true); }
}

}  // namespace volc
