#include "engine/resource.hpp"

#include "base/testing.hpp"

namespace volcano {

TEST_CASE("Application") {
  Application application{"test-app", 0};
  auto instance = application.create_instance();

  SECTION("ShouldPass") { REQUIRE(true); }
}

}  // namespace volcano
