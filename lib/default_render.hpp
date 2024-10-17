#pragma once

#include "lib/base.hpp"
#include "lib/render.hpp"

namespace volcano {

class DefaultRenderer final : public Renderer {
 public:
  DECLARE_COPY_DELETE(DefaultRenderer);
  DECLARE_MOVE_DEFAULT(DefaultRenderer);

  DefaultRenderer() = delete;
  virtual ~DefaultRenderer() = default;

  explicit DefaultRenderer(::VkSurfaceKHR surface) : Renderer{surface} {}

  bool HasSwapchain() const override { return has_swapchain_; }

  void RecreateSwapchain() override {}

  void Render() override {}

 private:
  bool has_swapchain_ = false;
};

}  // namespace volcano
