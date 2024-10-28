#pragma once

#include "lib/base.hpp"
#include "lib/render.hpp"
#include "vk/resource.hpp"

namespace volcano {

class SurfaceRenderer final : public Renderer {
 public:
  DECLARE_COPY_DELETE(SurfaceRenderer);
  DECLARE_MOVE_DEFAULT(SurfaceRenderer);

  SurfaceRenderer() = delete;
  virtual ~SurfaceRenderer() = default;

  explicit SurfaceRenderer(
      ::VkSurfaceKHR surface,
      const ::VkSurfaceCapabilitiesKHR& surface_capabilities,
      std::span<::VkSurfaceFormatKHR> surface_formats)
      : surface_{surface},
        surface_capabilities_{surface_capabilities},
        surface_formats_{surface_formats} {}

  bool HasSwapchain() const override { return has_swapchain_; }

  void RecreateSwapchain(::VkExtent2D geometry) override {
    const bool can_create_swapchain = {
        geometry.width >= surface_capabilities_().minImageExtent.width &&    //
        geometry.width <= surface_capabilities_().maxImageExtent.width &&    //
        geometry.width > 0 &&                                                //
        geometry.height >= surface_capabilities_().minImageExtent.height &&  //
        geometry.height <= surface_capabilities_().maxImageExtent.height &&  //
        geometry.height > 0};

    if (!can_create_swapchain) {
      has_swapchain_ = false;
      return;
    }

    // has_swapchain_ = swapchain_ != VK_NULL_HANDLE;
  }

  void Render() override {}

 private:
  ::VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  vk::PhysicalDeviceSurfaceCapabilities surface_capabilities_;
  vk::PhysicalDeviceSurfaceFormats surface_formats_;
  bool has_swapchain_ = false;
};

}  // namespace volcano
