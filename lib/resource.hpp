#pragma once

#include <string>
#include <string_view>

#include "lib/base.hpp"

namespace volc::lib {
namespace impl {
inline ::VkApplicationInfo MakeApplicationInfo() {
  return ::VkApplicationInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             .apiVersion = VK_API_VERSION_1_3};
}
inline ::VkInstanceCreateInfo MakeInstanceInfo() {
  return ::VkInstanceCreateInfo{.sType =
                                    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
}
inline ::VkDebugUtilsMessengerCreateInfoEXT MakeDebugInfo() {
  return ::VkDebugUtilsMessengerCreateInfoEXT{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
}
}  // namespace impl

class Application;

class Instance final {
 public:
  DECLARE_COPY_DELETE(Instance);
  DECLARE_MOVE_DEFAULT(Instance);

  Instance() = delete;
  ~Instance() = default;

 private:
  friend class Application;

  explicit Instance(const ::VkApplicationInfo* app_info,
                    const void* create_info_next = nullptr) {
    info_.pNext = create_info_next;
    info_.pApplicationInfo = app_info;
  }

  ::VkInstance instance_{};
  ::VkInstanceCreateInfo info_ = impl::MakeInstanceInfo();
};

class Application final {
 public:
  DECLARE_COPY_DELETE(Application);
  DECLARE_MOVE_DEFAULT(Application);

  enum class DebugSeverity { ERROR, WARNING, INFO, VERBOSE };

  explicit Application(std::string_view name, std::uint32_t version,
                       DebugSeverity debug_severity)
      : name_{name} {
    info_.pApplicationName = name_.c_str();
    info_.applicationVersion = version;

    debug_.messageSeverity = ConvertDebugSeverity(debug_severity);
  }

  Application() = delete;
  ~Application() = default;

  Instance CreateInstance() {
    return Instance{std::addressof(info_), std::addressof(debug_)};
  }

 private:
  ::VkApplicationInfo info_ = impl::MakeApplicationInfo();
  ::VkDebugUtilsMessengerCreateInfoEXT debug_ = impl::MakeDebugInfo();
  std::string name_;

  ::VkDebugUtilsMessageSeverityFlagsEXT ConvertDebugSeverity(DebugSeverity _) {
    switch (_) {
      case DebugSeverity::ERROR:
        return ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      case DebugSeverity::WARNING:
        return ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
               ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      case DebugSeverity::INFO:
        return ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
               ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
               ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      case DebugSeverity::VERBOSE:
        return ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
               ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
               ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
               ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    }
    CHECK_UNREACHABLE();
    return {};
  }
};

}  // namespace volc::lib
