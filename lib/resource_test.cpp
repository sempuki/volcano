#include "lib/resource.hpp"

#include "lib/testing.hpp"

namespace volc::lib {

TEST_CASE("Resources") {
  Factory factory;

  SECTION("ShouldPass") {
    REQUIRE(true);
  }
}

}  // namespace volc::lib
