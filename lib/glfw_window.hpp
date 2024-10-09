#pragma once

#include <queue>

#include "lib/base.hpp"
#include "lib/render.hpp"
#include "lib/window.hpp"

// NOTE: Do not include OpenGL headers.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace volc::glfw {

class PlatformWindow;

namespace impl {

class StaticInitialization final {
 public:
  DECLARE_COPY_DELETE(StaticInitialization);
  DECLARE_MOVE_DELETE(StaticInitialization);

  StaticInitialization() {
    ::glfwSetErrorCallback(ErrorCallback);
    initialized_ = ::glfwInit() == GLFW_TRUE &&  //
                   ::glfwVulkanSupported() == GLFW_TRUE;
    CHECK_POSTCONDITION(initialized_);
  }

  ~StaticInitialization() {
    ::glfwSetErrorCallback(nullptr);
    ::glfwTerminate();
  }

  bool IsInitialized() const { return initialized_; }

  static StaticInitialization& Instance() {
    static StaticInitialization instance_;
    return instance_;
  }

 private:
  bool initialized_ = false;

  struct Error {
    int error = 0;
    std::string description;
  };

  static std::queue<Error>& pending_errors() {
    static std::queue<Error> pending_errors_;
    return pending_errors_;
  }

  static void ErrorCallback(int error, const char* description) {
    pending_errors().push({.error = error, .description = description});
  }
};

class StaticState final {
 public:
  DECLARE_COPY_DELETE(StaticState);
  DECLARE_MOVE_DELETE(StaticState);

  StaticState() = default;
  ~StaticState() = default;

  static StaticState& Instance() {
    static StaticState instance_;
    return instance_;
  }

  bool Link(::GLFWwindow* glfw_window, PlatformWindow* platform_window) {
    return map_.try_emplace(glfw_window, platform_window).second;
  }

  PlatformWindow* Find(::GLFWwindow* glfw_window) const {
    auto iter = map_.find(glfw_window);
    return iter != map_.end() ? iter->second : nullptr;
  }

 private:
  std::unordered_map<::GLFWwindow*, PlatformWindow*> map_;
};

}  // namespace impl

class PlatformWindow final : public Window {
  using BaseType = Window;

 public:
  DECLARE_COPY_DELETE(PlatformWindow);
  DECLARE_MOVE_DELETE(PlatformWindow);

  PlatformWindow() = delete;
  ~PlatformWindow() = default;

  explicit PlatformWindow(std::string_view title, Window::Geometry geometry)
      : BaseType{title, geometry} {
    CHECK_PRECONDITION(impl::StaticInitialization::Instance().IsInitialized());

    ::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    ::glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    glfw_window_ = ::glfwCreateWindow(static_cast<int>(geometry_.width),
                                      static_cast<int>(geometry_.height),
                                      title_.c_str(), nullptr, nullptr);
    CHECK_POSTCONDITION(glfw_window_);

    bool inserted = impl::StaticState::Instance().Link(glfw_window_, this);
    CHECK_INVARIANT(inserted);

    ::glfwSetInputMode(glfw_window_, GLFW_STICKY_KEYS, GLFW_TRUE);
    ::glfwSetFramebufferSizeCallback(glfw_window_, FrameBufferSizeCallback);
    ::glfwSetWindowRefreshCallback(glfw_window_, WindowRefreshCallback);
    ::glfwSetKeyCallback(glfw_window_, KeyCallback);
  }

  std::span<const char*> RequiredExtensions() const {
    std::uint32_t count = 0;
    const char** extensions =
        ::glfwGetRequiredInstanceExtensions(std::addressof(count));
    return {extensions, count};
  }

  ::VkSurfaceKHR CreateSurface(::VkInstance instance) {
    CHECK_PRECONDITION(instance != VK_NULL_HANDLE);
    CHECK_INVARIANT(glfw_window_);

    ::VkSurfaceKHR surface = VK_NULL_HANDLE;
    ::VkResult result =
        ::glfwCreateWindowSurface(instance, glfw_window_, nullptr, &surface);
    CHECK_POSTCONDITION(result == VK_SUCCESS);
    CHECK_POSTCONDITION(surface != VK_NULL_HANDLE);

    return surface;
  }

 private:
  ::GLFWwindow* glfw_window_ = nullptr;

  static void FrameBufferSizeCallback(  //
      ::GLFWwindow* window,             //
      int width, int height) {
    PlatformWindow* platform_window =
        impl::StaticState::Instance().Find(window);

    CHECK_INVARIANT(platform_window);
    platform_window->renderer().RecreateSwapchain();
  }

  static void WindowRefreshCallback(  //
      ::GLFWwindow* window) {
    PlatformWindow* platform_window =
        impl::StaticState::Instance().Find(window);

    CHECK_INVARIANT(platform_window);
    platform_window->renderer().Render();
  }

  static void KeyCallback(   //
      ::GLFWwindow* window,  //
      int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
      ::glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
  }
};

}  // namespace volc::glfw