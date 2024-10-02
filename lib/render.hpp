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

  virtual bool has_swapchain() = 0;
  virtual bool recreate_swapchain() = 0;
  virtual bool render() = 0;

 protected:
};
}  // namespace volc
