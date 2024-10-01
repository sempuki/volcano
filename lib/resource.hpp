#pragma once

#include <string>
#include <string_view>
#include <vector>

#include "lib/base.hpp"

namespace volc::lib {
namespace impl {

inline auto MakeApplicationInfo() {
  return ::VkApplicationInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             .apiVersion = VK_API_VERSION_1_3};
}
inline auto MakeInstanceInfo() {
  return ::VkInstanceCreateInfo{.sType =
                                    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
}
inline auto MakeDebugInfo() {
  return ::VkDebugUtilsMessengerCreateInfoEXT{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT};
}

template <typename EnumeratorType, typename PropertyType>
inline void MaybeEnumerateProperties(
    EnumeratorType&& enumerate, InOut<std::vector<PropertyType>> properties) {
  std::uint32_t count = 0;
  std::vector<PropertyType> current;

  // Gather the required array size.
  ::VkResult result = enumerate(std::addressof(count), nullptr);
  CHECK_INVARIANT(result == VK_SUCCESS);
  current.resize(count);

  // Gather the enumerated properties.
  if (count) {
    result = enumerate(std::addressof(count), current.data());
    CHECK_INVARIANT(result == VK_SUCCESS);
    properties->insert(properties->end(),
                       std::make_move_iterator(current.begin()),
                       std::make_move_iterator(current.end()));
  }
}

template <typename PropertyType, typename PredicateType>
inline const PropertyType* TryFindPropertyIf(
    const std::vector<PropertyType>& properties, PredicateType&& predicate) {
  auto iter = std::find_if(properties.begin(), properties.end(), predicate);
  if (iter != properties.end()) {
    return std::addressof(*iter);
  }
  return nullptr;
}

inline bool HasLayer(const std::vector<::VkLayerProperties>& properties,
                     std::string_view layer_name) {
  return TryFindPropertyIf(
      properties, [layer_name](const ::VkLayerProperties& property) {
        return static_cast<std::string_view>(property.layerName) == layer_name;
      });
}

constexpr const char* VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";

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
                    std::vector<const char*> layers,
                    std::vector<const char*> extensions,
                    const void* create_info_next = nullptr)
      : layers_{std::move(layers)}, extensions_{std::move(extensions)} {
    info_.pNext = create_info_next;
    info_.pApplicationInfo = app_info;
    info_.enabledLayerCount = layers_.size();
    info_.ppEnabledLayerNames = layers_.data();
    info_.enabledExtensionCount = extensions_.size();
    info_.ppEnabledExtensionNames = extensions_.data();

    ::VkResult result = ::vkCreateInstance(std::addressof(info_), nullptr,
                                           std::addressof(instance_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

  ::VkInstance instance_ = nullptr;
  ::VkInstanceCreateInfo info_;
  std::vector<const char*> layers_;
  std::vector<const char*> extensions_;
};

class Application final {
 public:
  DECLARE_COPY_DELETE(Application);
  DECLARE_MOVE_DEFAULT(Application);

  enum class DebugSeverity { ERROR, WARNING, INFO, VERBOSE };

  explicit Application(std::string_view name, std::uint32_t version,
                       DebugSeverity severity)
      : name_{name} {
    impl::MaybeEnumerateProperties(::vkEnumerateInstanceLayerProperties,
                                   InOut(supported_layers_));
    for (auto&& property : supported_layers_) {
      impl::MaybeEnumerateProperties(
          [layer_name = property.layerName](
              std::uint32_t* count, ::VkExtensionProperties* properties) {
            return ::vkEnumerateInstanceExtensionProperties(  //
                layer_name, count, properties);
          },
          InOut(supported_extensions_));
    }

    for (auto&& property : supported_layers_) {
      std::cout << "Supported Instance Layer: " << property.layerName
                << std::endl;
    }

    for (auto&& property : supported_extensions_) {
      std::cout << "Supported Instance Extension: " << property.extensionName
                << std::endl;
    }

    // Instance extensions:
    // VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    debug_.messageSeverity = ConvertDebugSeverity(severity);
    debug_.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                         VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    info_.pNext = std::addressof(debug_);
    info_.pApplicationName = name_.c_str();
    info_.applicationVersion = version;
  }

  Application() = delete;
  ~Application() = default;

  Instance CreateInstance() {
    std::vector<const char*> requested_layers;
    std::vector<const char*> requested_extensions;

    if (impl::HasLayer(supported_layers_, impl::VALIDATION_LAYER_NAME)) {
      requested_layers.push_back(impl::VALIDATION_LAYER_NAME);
    }

    return Instance{std::addressof(info_),            //
                    std::move(requested_layers),      //
                    std::move(requested_extensions),  //
                    std::addressof(debug_)};
  }

 private:
  ::VkApplicationInfo info_ = impl::MakeApplicationInfo();
  ::VkDebugUtilsMessengerCreateInfoEXT debug_ = impl::MakeDebugInfo();
  std::string name_;
  std::vector<VkLayerProperties> supported_layers_;
  std::vector<VkExtensionProperties> supported_extensions_;

  static ::VkDebugUtilsMessageSeverityFlagsEXT ConvertDebugSeverity(
      DebugSeverity _) {
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
