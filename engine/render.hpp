#pragma once

#include "engine/base.hpp"
#include "vk/resource.hpp"

namespace volcano {

class Renderer {
 public:
  DECLARE_COPY_DELETE(Renderer);
  DECLARE_MOVE_DEFAULT(Renderer);

  Renderer() = default;

  virtual ~Renderer() = default;
  virtual auto HasSwapchain() const -> bool = 0;
  virtual auto RecreateSwapchain(::VkExtent2D geometry) -> void = 0;
  virtual auto Render() -> void = 0;
};

}  // namespace volcano
