#pragma once

#include "lib/base.hpp"
#include "vk/resource.hpp"

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

  void set_renderer(std::unique_ptr<Renderer> renderer) {
    renderer_ = std::move(renderer);
  }

  const Geometry& geometry() const { return geometry_; }

  virtual std::span<const char*> required_extensions() const = 0;
  virtual ::VkSurfaceKHR create_surface(::VkInstance instance) = 0;
  virtual void show() = 0;

 protected:
  Renderer& renderer() {
    CHECK_PRECONDITION(renderer_);
    return *renderer_;
  }

  std::string title_;
  Geometry geometry_;

 private:
  std::unique_ptr<Renderer> renderer_;
};

}  // namespace volcano
