#pragma once

#include "engine/base.hpp"
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

  auto set_renderer(std::unique_ptr<Renderer> renderer) -> void {
    renderer_ = std::move(renderer);
  }

  auto geometry() const -> const Geometry& { return geometry_; }

  virtual auto required_extensions() const -> std::span<const char*> = 0;
  virtual auto create_surface(::VkInstance instance) -> ::VkSurfaceKHR = 0;
  virtual auto show() -> void = 0;

 protected:
  auto renderer() -> Renderer& {
    CHECK_PRECONDITION(renderer_);
    return *renderer_;
  }

  std::string title_;
  Geometry geometry_;

 private:
  std::unique_ptr<Renderer> renderer_;
};

}  // namespace volcano
