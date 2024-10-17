#include "lib/resource.hpp"

#include "lib/testing.hpp"

namespace volcano {

TEST_CASE("Application") {
  Application application{"test-app", 0};
  auto instance = application.CreateInstance();

  SECTION("ShouldPass") { REQUIRE(true); }
}

}  // namespace volcano
