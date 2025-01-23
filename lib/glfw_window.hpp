#pragma once

#include <vector>

#include "lib/base.hpp"
#include "lib/render.hpp"
#include "lib/window.hpp"
#include "vk/resource.hpp"

// NOTE: Do not include OpenGL headers.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace volcano::glfw {

class PlatformWindow;

namespace impl {

class StaticState final {
 public:
  DECLARE_COPY_DELETE(StaticState);
  DECLARE_MOVE_DELETE(StaticState);

  StaticState() = default;
  ~StaticState() = default;

  static StaticState& instance() {
    static StaticState instance_;
    return instance_;
  }

  bool link(::GLFWwindow* glfw_window, PlatformWindow* platform_window) {
    return map_.try_emplace(glfw_window, platform_window).second;
  }

  PlatformWindow* find(::GLFWwindow* glfw_window) const {
    auto iter = map_.find(glfw_window);
    return iter != map_.end() ? iter->second : nullptr;
  }

  void raise_error(int code, const char* description) {
    pending_errors_.push_back({
        .code = code,               //
        .description = description  //
    });
  }

  bool has_pending_errors() const { return pending_errors_.size(); }
  void dump_pending_errors() const {
    for (auto&& error : pending_errors_) {
      std::print("[ERROR] <GLFW> {}: {}\n", error.description, error.code);
    }
  }

 private:
  struct Error final {
    int code = 0;
    std::string description;
  };

  std::unordered_map<::GLFWwindow*, PlatformWindow*> map_;
  std::vector<Error> pending_errors_;
};

class StaticInitialization final {
 public:
  DECLARE_COPY_DELETE(StaticInitialization);
  DECLARE_MOVE_DELETE(StaticInitialization);

  StaticInitialization() {
    ::glfwSetErrorCallback(error_callback);
    initialized_ = ::glfwInit() == GLFW_TRUE &&  //
                   ::glfwVulkanSupported() == GLFW_TRUE;
    CHECK_POSTCONDITION(initialized_);
  }

  ~StaticInitialization() {
    ::glfwSetErrorCallback(nullptr);
    ::glfwTerminate();
  }

  bool is_initialized() const { return initialized_; }

  static StaticInitialization& instance() {
    static StaticInitialization instance_;
    return instance_;
  }

 private:
  bool initialized_ = false;

  static void error_callback(int code, const char* description) {
    StaticState::instance().raise_error(code, description);
  }
};

}  // namespace impl

class PlatformWindow final : public Window {
  using BaseType = Window;

 public:
  DECLARE_COPY_DELETE(PlatformWindow);
  DECLARE_MOVE_DELETE(PlatformWindow);

  PlatformWindow() = delete;
  ~PlatformWindow() {
    if (glfw_window_) {
      ::glfwDestroyWindow(glfw_window_);
    }
  }

  explicit PlatformWindow(std::string_view title, Window::Geometry geometry)
      : BaseType{title, geometry} {
    CHECK_PRECONDITION(impl::StaticInitialization::instance().is_initialized());

    ::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    ::glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    glfw_window_ = ::glfwCreateWindow(static_cast<int>(geometry_.width),
                                      static_cast<int>(geometry_.height),
                                      title_.c_str(), nullptr, nullptr);
    CHECK_POSTCONDITION(glfw_window_);

    bool inserted = impl::StaticState::instance().link(glfw_window_, this);
    CHECK_INVARIANT(inserted);

    ::glfwSetInputMode(glfw_window_, GLFW_STICKY_KEYS, GLFW_TRUE);
    ::glfwSetFramebufferSizeCallback(glfw_window_, frame_buffer_size_callback);
    ::glfwSetWindowRefreshCallback(glfw_window_, window_refresh_callback);
    ::glfwSetKeyCallback(glfw_window_, key_callback);
  }

  std::span<const char*> required_extensions() const override {
    std::uint32_t count = 0;
    const char** extensions =
        ::glfwGetRequiredInstanceExtensions(std::addressof(count));
    return {extensions, count};
  }

  ::VkSurfaceKHR create_surface(::VkInstance instance) override {
    CHECK_PRECONDITION(instance != VK_NULL_HANDLE);
    CHECK_INVARIANT(glfw_window_);

    ::VkSurfaceKHR surface = VK_NULL_HANDLE;
    ::VkResult result =
        ::glfwCreateWindowSurface(instance, glfw_window_, nullptr, &surface);
    CHECK_POSTCONDITION(result == VK_SUCCESS);
    CHECK_POSTCONDITION(surface != VK_NULL_HANDLE);

    return surface;
  }

  void show() override {
    ::glfwShowWindow(glfw_window_);

    if (!renderer().HasSwapchain()) {
      int width = 0, height = 0;
      ::glfwGetWindowSize(glfw_window_, &width, &height);
      CHECK_PRECONDITION(width > 0 && height > 0)

      renderer().RecreateSwapchain(
          {.width = static_cast<std::uint32_t>(width),
           .height = static_cast<std::uint32_t>(height)});
    }

    while (!impl::StaticState::instance().has_pending_errors() &&
           !::glfwWindowShouldClose(glfw_window_)) {
      if (renderer().HasSwapchain()) {
        ::glfwPollEvents();  // Non-blocking.
      } else {
        ::glfwWaitEvents();  // Blocking.
      }

      if (renderer().HasSwapchain()) {
        renderer().Render();
      }
    }

    impl::StaticState::instance().dump_pending_errors();
  }

 private:
  ::GLFWwindow* glfw_window_ = nullptr;

  static void frame_buffer_size_callback(  //
      ::GLFWwindow* window,                //
      int width, int height) {
    PlatformWindow* platform_window =
        impl::StaticState::instance().find(window);
    CHECK_INVARIANT(platform_window);

    CHECK_PRECONDITION(width > 0 && height > 0)
    platform_window->renderer().RecreateSwapchain(
        {.width = static_cast<std::uint32_t>(width),
         .height = static_cast<std::uint32_t>(height)});
  }

  static void window_refresh_callback(  //
      ::GLFWwindow* window) {
    PlatformWindow* platform_window =
        impl::StaticState::instance().find(window);

    CHECK_INVARIANT(platform_window);
    platform_window->renderer().Render();
  }

  static void key_callback(  //
      ::GLFWwindow* window,  //
      int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      ::glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
  }
};

}  // namespace volcano::glfw
