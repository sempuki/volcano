#pragma once

#include <queue>

#include "lib/base.hpp"

namespace volc {

class Renderer;

class Window {
 public:
  struct Geometry {
    std::size_t width = 0;
    std::size_t height = 0;
  };

  DECLARE_COPY_DELETE(Window);
  DECLARE_MOVE_DELETE(Window);

  Window() = delete;
  virtual ~Window() = default;

  explicit Window(std::string_view title, Geometry geometry)
      : title_{title}, geometry_{geometry} {}

  std::string_view Title() const { return title_; }
  Geometry Geometry() const { return geometry_; }

  void SetRenderer(std::unique_ptr<Renderer> renderer) {
    renderer_ = std::move(renderer);
  }

 protected:
  Renderer& Renderer() {
    CHECK_PRECONDITION(renderer_);
    return *renderer;
  }

  std::string title_;
  Geometry geometry_;

 private:
  std::unique_ptr<Renderer> renderer_;
};

}  // namespace volc
