#include "engine/glfw_window.hpp"
#include "engine/resource.hpp"

#include "base/testing.hpp"

namespace volcano {

TEST_CASE("Integration") {
  Application application{"test-app", 0};
  glfw::PlatformWindow platform_window{"test-glfw-window",
                                       {.width = 800, .height = 600}};

  auto instance = application.create_instance(
      {}, platform_window.required_extensions(), DebugLevel::VERBOSE);
  auto surface = platform_window.create_surface(instance);
  auto device = instance.create_presentation_device(surface);

  SECTION("ShouldPass") { REQUIRE(true); }
}

}  // namespace volcano
