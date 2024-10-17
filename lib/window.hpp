#pragma once

#include "lib/base.hpp"

namespace volcano {

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

  void SetRenderer(std::unique_ptr<Renderer> renderer) {
    renderer_ = std::move(renderer);
  }

  virtual std::span<const char*> RequiredExtensions() const = 0;
  virtual ::VkSurfaceKHR CreateSurface(::VkInstance instance) = 0;
  virtual void Show() = 0;

 protected:
  Renderer& GetRenderer() {
    CHECK_PRECONDITION(renderer_);
    return *renderer_;
  }

  std::string title_;
  Geometry geometry_;

 private:
  std::unique_ptr<Renderer> renderer_;
};

}  // namespace volcano
