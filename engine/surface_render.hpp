#pragma once

#include <limits>

#include "engine/base.hpp"
#include "engine/render.hpp"
#include "vk/resource.hpp"

namespace volcano {

class SurfaceRenderer final : public Renderer {
 public:
  DECLARE_COPY_DELETE(SurfaceRenderer);
  DECLARE_MOVE_DEFAULT(SurfaceRenderer);

  SurfaceRenderer() = delete;
  virtual ~SurfaceRenderer() = default;

  template <typename DoRecreateSwapchainType, typename DoRenderType>
  explicit SurfaceRenderer(                          //
      ::VkPhysicalDevice phys_device,                //
      ::VkSurfaceKHR surface,                        //
      DoRecreateSwapchainType&& recreate_swapchain,  //
      DoRenderType&& render)
      : phys_device_{phys_device},
        surface_{surface},
        render_{std::forward<DoRenderType>(render)},
        recreate_swapchain_{
            std::forward<DoRecreateSwapchainType>(recreate_swapchain)} {
    CHECK_POSTCONDITION(render_);
    CHECK_POSTCONDITION(recreate_swapchain_);
  }

  auto HasSwapchain() const -> bool override { return has_swapchain_; }

  auto RecreateSwapchain(::VkExtent2D geometry) -> void override {
    vk::PhysicalDeviceSurfaceCapabilities surface_capabilities{phys_device_,
                                                               surface_};

    const bool can_create_swapchain = {
        geometry.width >= surface_capabilities().minImageExtent.width &&    //
        geometry.width <= surface_capabilities().maxImageExtent.width &&    //
        geometry.width > 0 &&                                               //
        geometry.height >= surface_capabilities().minImageExtent.height &&  //
        geometry.height <= surface_capabilities().maxImageExtent.height &&  //
        geometry.height > 0 &&                                              //
        // A defined current extent of 0x0 means the surface is minimized.
        (surface_capabilities().currentExtent.width ==
             std::numeric_limits<std::uint32_t>::max() ||
         (surface_capabilities().currentExtent.width > 0 &&
          surface_capabilities().currentExtent.height > 0))};

    if (!can_create_swapchain) {
      has_swapchain_ = false;
      return;
    }

    has_swapchain_ = recreate_swapchain_(geometry);
  }

  auto Render() -> void override {
    if (has_swapchain_) {
      CHECK_INVARIANT(render_);
      render_();
    }
  }

 private:
  ::VkPhysicalDevice phys_device_ = VK_NULL_HANDLE;
  ::VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  bool has_swapchain_ = false;

  std::move_only_function<void()> render_;
  std::move_only_function<bool(::VkExtent2D)> recreate_swapchain_;
};

}  // namespace volcano
