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
  std::print("1111\n");
  auto surface = platform_window.CreateSurface(instance.Handle());
  std::print("2222\n");
  auto device = instance.FindDeviceForSurface(surface);
  std::print("3333\n");

  SECTION("ShouldPass") { REQUIRE(true); }
}

}  // namespace volc
