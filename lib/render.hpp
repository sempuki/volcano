#pragma once

#include <vector>

#include "lib/base.hpp"

namespace volc {

class Renderer {
 public:
  DECLARE_COPY_DELETE(Renderer);
  DECLARE_MOVE_DEFAULT(Renderer);

  Renderer() = delete;
  virtual ~Renderer() = default;

  virtual bool HasSwapchain() = 0;
  virtual void RecreateSwapchain() = 0;
  virtual void SetSurface(::VkSurfaceKHR surface) = 0;
  virtual void Render() = 0;

 protected:
};
}  // namespace volc
