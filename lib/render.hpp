#pragma once

#include "lib/base.hpp"

namespace volcano {

class Renderer {
 public:
  DECLARE_COPY_DELETE(Renderer);
  DECLARE_MOVE_DEFAULT(Renderer);

  Renderer() = default;

  virtual ~Renderer() = default;
  virtual bool HasSwapchain() const = 0;
  virtual void RecreateSwapchain() = 0;
  virtual void Render() = 0;
};

}  // namespace volcano
