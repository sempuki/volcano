#pragma once

#include <algorithm>
#include <functional>
#include <iostream>
#include <map>
#include <sstream>
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
  InvokeWithContinuation(
      Overloaded(
          [](::VkResult result) { CHECK_INVARIANT(result == VK_SUCCESS); },
          []() {}),
      enumerate, std::addressof(count), nullptr);
  current.resize(count);

  // Gather the enumerated properties.
  if (count) {
    InvokeWithContinuation(
        Overloaded(
            [](::VkResult result) { CHECK_INVARIANT(result == VK_SUCCESS); },
            []() {}),
        enumerate, std::addressof(count), current.data());

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
  CHECK_PRECONDITION(instance != VK_NULL_HANDLE);
  *target = reinterpret_cast<TargetFunctionPointer>(
      ::vkGetInstanceProcAddr(instance, name));
}

inline std::string ConvertToString(::VkPhysicalDeviceType _) {
  switch (_) {
    case VK_PHYSICAL_DEVICE_TYPE_OTHER:
      return "VK_PHYSICAL_DEVICE_TYPE_OTHER";
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
      return "VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
      return "VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
      return "VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
      return "VK_PHYSICAL_DEVICE_TYPE_CPU";
  }
  CHECK_UNREACHABLE();
  return {};
}

inline std::string ConvertToString(::VkQueueFlagBits _) {
  switch (_) {
    case VK_QUEUE_GRAPHICS_BIT:
      return "VK_QUEUE_GRAPHICS_BIT";
    case VK_QUEUE_COMPUTE_BIT:
      return "VK_QUEUE_COMPUTE_BIT";
    case VK_QUEUE_TRANSFER_BIT:
      return "VK_QUEUE_TRANSFER_BIT";
    case VK_QUEUE_SPARSE_BINDING_BIT:
      return "VK_QUEUE_SPARSE_BINDING_BIT";
    case VK_QUEUE_PROTECTED_BIT:
      return "VK_QUEUE_PROTECTED_BIT";
    case VK_QUEUE_VIDEO_DECODE_BIT_KHR:
      return "VK_QUEUE_VIDEO_DECODE_BIT_KHR";
    case VK_QUEUE_VIDEO_ENCODE_BIT_KHR:
      return "VK_QUEUE_VIDEO_ENCODE_BIT_KHR";
  }
  CHECK_UNREACHABLE();
  return {};
}

inline std::string ConvertToString(::VkQueueFlags flags) {
  std::stringstream stream;
  for (auto bit :
       {VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT,
        VK_QUEUE_SPARSE_BINDING_BIT, VK_QUEUE_PROTECTED_BIT,
        VK_QUEUE_VIDEO_DECODE_BIT_KHR, VK_QUEUE_VIDEO_ENCODE_BIT_KHR}) {
    if (flags & bit) {
      stream << ConvertToString(static_cast<::VkQueueFlagBits>(flags & bit))
             << ", ";
    }
  }
  return std::move(stream).str();
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
    if (instance_ != VK_NULL_HANDLE) {
      if (destroy_debug_messenger_ && debug_messenger_ != VK_NULL_HANDLE) {
        destroy_debug_messenger_(instance_, debug_messenger_, impl::ALLOCATOR);
      }
      ::vkDestroyInstance(instance_, impl::ALLOCATOR);
    }
  }

  ::VkPhysicalDevice FindDeviceForSurface(::VkSurfaceKHR surface) {
    CHECK_PRECONDITION(surface != VK_NULL_HANDLE);

    for (auto&& device : devices_) {
      auto&& device_property = device_properties_[device];

      if (device_property.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        auto&& queue_family_properties = queue_family_properties_[device];

        for (std::size_t queue_family_i = 0;
             queue_family_i < queue_family_properties.size();
             ++queue_family_i) {
          ::VkBool32 supported = VK_FALSE;
          ::VkResult result = ::vkGetPhysicalDeviceSurfaceSupportKHR(
              device, queue_family_i, surface, std::addressof(supported));
          CHECK_POSTCONDITION(result == VK_SUCCESS);

          if (supported) {
            return device;
          }
        }
      }
    }

    return VK_NULL_HANDLE;
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
        CHECK_POSTCONDITION(debug_messenger_ != VK_NULL_HANDLE);
      } else {
        std::cerr << "Missing debug extension.";
      }
    }

    impl::MaybeEnumerateProperties(
        std::bind_front(::vkEnumeratePhysicalDevices, instance_),
        InOut(devices_));

    for (auto&& device : devices_) {
      ::vkGetPhysicalDeviceProperties(
          device, std::addressof(device_properties_[device]));
      std::cout << "Physical Device Name: "
                << device_properties_[device].deviceName << " ["
                << impl::ConvertToString(device_properties_[device].deviceType)
                << "]\n";

      impl::MaybeEnumerateProperties(
          std::bind_front(::vkGetPhysicalDeviceQueueFamilyProperties, device),
          InOut(queue_family_properties_[device]));

      std::cout << "Queue Family Flags: \n";
      for (auto&& property : queue_family_properties_[device]) {
        std::cout << "--  [" << property.queueCount << "] "
                  << impl::ConvertToString(property.queueFlags) << '\n';
      }
    }
  }

  ::VkInstance instance_ = VK_NULL_HANDLE;
  ::VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;

  ::PFN_vkSubmitDebugUtilsMessageEXT submit_debug_message_ = nullptr;
  ::PFN_vkCreateDebugUtilsMessengerEXT create_debug_messenger_ = nullptr;
  ::PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_messenger_ = nullptr;

  ::VkInstanceCreateInfo instance_info_ = impl::MakeInstanceCreateInfo();
  ::VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info_ =
      impl::MakeDebugMessengerCreateInfo();

  std::vector<const char*> layers_;
  std::vector<const char*> extensions_;
  std::vector<::VkPhysicalDevice> devices_;
  std::map<::VkPhysicalDevice, ::VkPhysicalDeviceProperties> device_properties_;
  std::map<::VkPhysicalDevice, std::vector<::VkQueueFamilyProperties>>
      queue_family_properties_;

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
      const ::VkDebugUtilsMessengerCallbackDataEXT* data, void*) {
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
    return VK_FALSE;
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
          std::bind_front(::vkEnumerateInstanceExtensionProperties,
                          property.layerName),
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
