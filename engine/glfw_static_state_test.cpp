#include "engine/glfw_window.hpp"

#include "base/testing.hpp"

namespace volcano::glfw {

// StaticState only maps pointers, so it needs neither glfwInit nor a display.
TEST_CASE("StaticState") {
  auto& state = internal::StaticState::instance();
  auto* glfw_window = reinterpret_cast<::GLFWwindow*>(0x1000);
  auto* first = reinterpret_cast<PlatformWindow*>(0x2000);
  auto* second = reinterpret_cast<PlatformWindow*>(0x3000);

  SECTION("ShouldRelinkGivenAddressReusedAfterUnlink") {
    REQUIRE(state.link(glfw_window, first));
    state.unlink(glfw_window);
    REQUIRE(state.find(glfw_window) == nullptr);

    REQUIRE(state.link(glfw_window, second));
    REQUIRE(state.find(glfw_window) == second);
    state.unlink(glfw_window);
  }
}

}  // namespace volcano::glfw
