#include "lib/glfw_window.hpp"
#include "lib/resource.hpp"

#include "lib/testing.hpp"

namespace volc {

TEST_CASE("Integration") {
  Application application{"test-app", 0};
  glfw::PlatformWindow platform_window{"test-glfw-window",
                                       {.width = 800, .height = 600}};

  auto instance = application.CreateInstance(
      {}, platform_window.RequiredExtensions(), DebugLevel::VERBOSE);
  auto surface = platform_window.CreateSurface(instance.Handle());
  auto device = instance.FindDeviceForSurface(surface);

  SECTION("ShouldPass") { REQUIRE(true); }
}

}  // namespace volc
