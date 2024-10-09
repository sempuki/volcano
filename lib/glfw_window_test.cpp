#include "lib/glfw_window.hpp"

#include "lib/testing.hpp"

namespace volc::glfw {

TEST_CASE("GlfwWindow") {
  PlatformWindow platform_window{"test-glfw-window",
                                 {.width = 800, .height = 600}};

  SECTION("ShouldPass") { REQUIRE(true); }
}

}  // namespace volc::glfw
