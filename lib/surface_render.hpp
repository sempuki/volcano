#pragma once

#include "lib/base.hpp"
#include "lib/render.hpp"

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
      std::vector<::VkSurfaceFormatKHR> surface_formats)
      : surface_{surface},
        surface_capabilities_{surface_capabilities},
        surface_formats_{std::move(surface_formats)} {}

  bool HasSwapchain() const override { return has_swapchain_; }

  void RecreateSwapchain(::VkExtent2D geometry) override {}

  void Render() override {}

 private:
  ::VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  ::VkSurfaceCapabilitiesKHR surface_capabilities_;
  std::vector<::VkSurfaceFormatKHR> surface_formats_;
  bool has_swapchain_ = false;
};

}  // namespace volcano
