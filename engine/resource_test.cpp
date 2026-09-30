#include "engine/resource.hpp"

#include <limits>

#include "base/testing.hpp"

namespace volcano {

TEST_CASE("Application") {
  Application application{"test-app", 0};
  auto instance = application.create_instance();

  SECTION("ShouldPass") { REQUIRE(true); }
}

TEST_CASE("ChooseSwapchainImageCount") {
  ::VkSurfaceCapabilitiesKHR capabilities{};

  SECTION("ShouldBeOneMoreThanMinimumGivenRoom") {
    capabilities.minImageCount = 2;
    capabilities.maxImageCount = 4;
    REQUIRE(choose_swapchain_image_count(capabilities) == 3u);
  }

  SECTION("ShouldBeMaximumGivenMinimumEqualsMaximum") {
    capabilities.minImageCount = 3;
    capabilities.maxImageCount = 3;
    REQUIRE(choose_swapchain_image_count(capabilities) == 3u);
  }

  SECTION("ShouldBeOneMoreThanMinimumGivenNoMaximum") {
    capabilities.minImageCount = 2;
    capabilities.maxImageCount = 0;
    REQUIRE(choose_swapchain_image_count(capabilities) == 3u);
  }
}

TEST_CASE("ChooseSwapchainExtent") {
  ::VkSurfaceCapabilitiesKHR capabilities{
      .minImageExtent = {.width = 100, .height = 100},
      .maxImageExtent = {.width = 1000, .height = 1000},
  };

  SECTION("ShouldBeCurrentExtentGivenDefinedCurrentExtent") {
    capabilities.currentExtent = {.width = 640, .height = 480};
    auto extent =
        choose_swapchain_extent(capabilities, {.width = 1, .height = 1});
    REQUIRE(extent.width == 640u);
    REQUIRE(extent.height == 480u);
  }

  SECTION("ShouldClampRequestGivenUndefinedCurrentExtent") {
    capabilities.currentExtent = {
        .width = std::numeric_limits<std::uint32_t>::max(),
        .height = std::numeric_limits<std::uint32_t>::max(),
    };
    auto extent =
        choose_swapchain_extent(capabilities, {.width = 50, .height = 5000});
    REQUIRE(extent.width == 100u);
    REQUIRE(extent.height == 1000u);
  }
}

}  // namespace volcano
