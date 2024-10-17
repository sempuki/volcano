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

  explicit SurfaceRenderer(::VkSurfaceKHR surface) : surface_{surface} {}

  bool HasSwapchain() const override { return has_swapchain_; }

  void RecreateSwapchain() override {}

  void Render() override {}

 private:
  ::VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  bool has_swapchain_ = false;
};

}  // namespace volcano
