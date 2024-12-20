#pragma once

#include "lib/base.hpp"
#include "vk/resource.hpp"

namespace volcano {

class Renderer {
 public:
  DECLARE_COPY_DELETE(Renderer);
  DECLARE_MOVE_DEFAULT(Renderer);

  Renderer() = default;

  virtual ~Renderer() = default;
  virtual bool HasSwapchain() const = 0;
  virtual void RecreateSwapchain(::VkExtent2D geometry) = 0;
  virtual void Render() = 0;
};

}  // namespace volcano
