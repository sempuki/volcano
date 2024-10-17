#pragma once

#include "lib/base.hpp"

namespace volcano {

class Renderer {
 public:
  DECLARE_COPY_DELETE(Renderer);
  DECLARE_MOVE_DEFAULT(Renderer);

  Renderer() = delete;
  virtual ~Renderer() = default;

  explicit Renderer(::VkSurfaceKHR surface) : surface_{surface} {}

  virtual bool HasSwapchain() const = 0;
  virtual void RecreateSwapchain() = 0;
  virtual void Render() = 0;

 protected:
  ::VkSurfaceKHR surface_ = VK_NULL_HANDLE;
};

}  // namespace volcano
