#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

#include "lib/base.hpp"

namespace volc {
namespace impl {

inline auto MakeApplicationInfo() {
  return ::VkApplicationInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
                             .apiVersion = VK_API_VERSION_1_3};
}
inline auto MakeInstanceCreateInfo() {
  return ::VkInstanceCreateInfo{.sType =
                                    VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
}
inline auto MakeDebugMessengerCreateInfo() {
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
    properties->insert(properties->end(),  //
                       current.begin(),    //
                       current.end());
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

inline bool HasLayerProperty(const std::vector<::VkLayerProperties>& properties,
                             std::string_view layer_name) {
  return std::any_of(
      properties.begin(), properties.end(),
      [layer_name](const ::VkLayerProperties& property) {
        return static_cast<std::string_view>(property.layerName) == layer_name;
      });
}

inline bool HasStringName(const std::vector<const char*>& names,
                          std::string_view target) {
  return std::any_of(names.begin(), names.end(), [target](const char* name) {
    return static_cast<std::string_view>(name) == target;
  });
}

template <typename TargetFunctionPointer>
inline void LoadInstanceFunction(const char* name, ::VkInstance instance,
                                 Out<TargetFunctionPointer> target) {
  *target = reinterpret_cast<TargetFunctionPointer>(
      ::vkGetInstanceProcAddr(instance, name));
}

constexpr const char* VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";
constexpr const char* DEBUG_EXTENSION_NAME = "VK_EXT_debug_utils";
constexpr const char* DEBUG_CREATE_FUNCTION_NAME =
    "vkCreateDebugUtilsMessengerEXT";
constexpr const char* DEBUG_DESTROY_FUNCTION_NAME =
    "vkDestroyDebugUtilsMessengerEXT";
constexpr const char* DEBUG_SUBMIT_FUNCTION_NAME =
    "vkSubmitDebugUtilsMessageEXT";

const ::VkAllocationCallbacks* ALLOCATOR = nullptr;

}  // namespace impl

enum class DebugLevel {
  NONE,
  ERROR,
  WARNING,
  INFO,
  VERBOSE,
};

class Application;

class Instance final {
 public:
  DECLARE_COPY_DELETE(Instance);
  DECLARE_MOVE_DEFAULT(Instance);

  Instance() = delete;
  ~Instance() {
    if (destroy_debug_messenger_ && debug_messenger_) {
      destroy_debug_messenger_(instance_, debug_messenger_, impl::ALLOCATOR);
    }
    ::vkDestroyInstance(instance_, impl::ALLOCATOR);
  }

 private:
  friend class Application;

  explicit Instance(const ::VkApplicationInfo* app_info,  //
                    std::vector<const char*> layers,      //
                    std::vector<const char*> extensions,  //
                    DebugLevel debug_level)
      : layers_{std::move(layers)}, extensions_{std::move(extensions)} {
    instance_info_.pApplicationInfo = app_info;
    instance_info_.enabledLayerCount = layers_.size();
    instance_info_.ppEnabledLayerNames = layers_.data();
    instance_info_.enabledExtensionCount = extensions_.size();
    instance_info_.ppEnabledExtensionNames = extensions_.data();

    if (debug_level != DebugLevel::NONE) {
      if (impl::HasStringName(extensions_, impl::DEBUG_EXTENSION_NAME)) {
        instance_info_.pNext = std::addressof(debug_messenger_info_);
        debug_messenger_info_.messageSeverity =
            ConvertToDebugSeverity(debug_level);
        debug_messenger_info_.messageType =
            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        debug_messenger_info_.pfnUserCallback = DebugMessengerCallback;
      } else {
        std::cerr << "Missing debug extension.";
      }
    }

    ::VkResult result =
        ::vkCreateInstance(std::addressof(instance_info_), impl::ALLOCATOR,
                           std::addressof(instance_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);

    if (debug_level != DebugLevel::NONE) {
      if (impl::HasStringName(extensions_, impl::DEBUG_EXTENSION_NAME)) {
        impl::LoadInstanceFunction(impl::DEBUG_CREATE_FUNCTION_NAME, instance_,
                                   Out(create_debug_messenger_));
        impl::LoadInstanceFunction(impl::DEBUG_DESTROY_FUNCTION_NAME, instance_,
                                   Out(destroy_debug_messenger_));
        impl::LoadInstanceFunction(impl::DEBUG_SUBMIT_FUNCTION_NAME, instance_,
                                   Out(submit_debug_message_));

        ::VkResult result = create_debug_messenger_(
            instance_, std::addressof(debug_messenger_info_), impl::ALLOCATOR,
            std::addressof(debug_messenger_));
        CHECK_POSTCONDITION(result == VK_SUCCESS);
        CHECK_POSTCONDITION(debug_messenger_);
      } else {
        std::cerr << "Missing debug extension.";
      }
    }
  }

  ::VkInstance instance_ = nullptr;

  ::VkDebugUtilsMessengerEXT debug_messenger_ = nullptr;
  ::PFN_vkSubmitDebugUtilsMessageEXT submit_debug_message_ = nullptr;
  ::PFN_vkCreateDebugUtilsMessengerEXT create_debug_messenger_ = nullptr;
  ::PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_messenger_ = nullptr;

  ::VkInstanceCreateInfo instance_info_;
  ::VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info_ =
      impl::MakeDebugMessengerCreateInfo();

  std::vector<const char*> layers_;
  std::vector<const char*> extensions_;

  static ::VkDebugUtilsMessageSeverityFlagsEXT ConvertToDebugSeverity(
      DebugLevel _) {
    switch (_) {
      case DebugLevel::NONE:
        return 0;
      case DebugLevel::ERROR:
        return ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      case DebugLevel::WARNING:
        return ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
               ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      case DebugLevel::INFO:
        return ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
               ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
               ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
      case DebugLevel::VERBOSE:
        return ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
               ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
               ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
               ::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    }
    CHECK_UNREACHABLE();
    return {};
  }

  static VKAPI_ATTR ::VkBool32 DebugMessengerCallback(
      ::VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      ::VkDebugUtilsMessageTypeFlagsEXT message_types,
      const ::VkDebugUtilsMessengerCallbackDataEXT* data, void*) noexcept {
    CHECK_PRECONDITION(
        data->sType ==
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT);

    std::cout << "Message Id Name: " << data->pMessageIdName << std::endl;
    std::cout << "Message : " << data->pMessage << std::endl;
    std::cout << "Queue Labels: \n";
    for (std::uint32_t i = 0; i < data->queueLabelCount; ++i) {
      const ::VkDebugUtilsLabelEXT& _ = data->pQueueLabels[i];
      std::cout << "..  " << _.pLabelName << '\n';
    }
    std::cout << "Command Buffer Labels: \n";
    for (std::uint32_t i = 0; i < data->cmdBufLabelCount; ++i) {
      const ::VkDebugUtilsLabelEXT& _ = data->pCmdBufLabels[i];
      std::cout << "::  " << _.pLabelName << '\n';
    }
    std::cout << "Object Names: \n";
    for (std::uint32_t i = 0; i < data->objectCount; ++i) {
      const ::VkDebugUtilsObjectNameInfoEXT& _ = data->pObjects[i];
      std::cout << "**  " << _.pObjectName << '\n';
    }
  }
};

class Application final {
 public:
  DECLARE_COPY_DELETE(Application);
  DECLARE_MOVE_DEFAULT(Application);

  Application() = delete;
  ~Application() = default;

  explicit Application(std::string_view name, std::uint32_t version)
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

    info_.pApplicationName = name_.c_str();
    info_.applicationVersion = version;
  }

  Instance CreateInstance(DebugLevel debug_level = DebugLevel::NONE) {
    std::vector<const char*> requested_layers;
    std::vector<const char*> requested_extensions{
        VK_KHR_SURFACE_EXTENSION_NAME,  //
        // VK_KHR_XCB_SURFACE_EXTENSION_NAME,  //
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME  //
    };

    if (impl::HasLayerProperty(supported_layers_,
                               impl::VALIDATION_LAYER_NAME)) {
      requested_layers.push_back(impl::VALIDATION_LAYER_NAME);
    }

    return Instance{std::addressof(info_),            //
                    std::move(requested_layers),      //
                    std::move(requested_extensions),  //
                    debug_level};
  }

 private:
  ::VkApplicationInfo info_ = impl::MakeApplicationInfo();

  std::vector<VkLayerProperties> supported_layers_;
  std::vector<VkExtensionProperties> supported_extensions_;
  std::string name_;
};

}  // namespace volc
