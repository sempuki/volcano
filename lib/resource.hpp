#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <functional>
#include <map>
#include <sstream>
#include <vector>

#include "lib/base.hpp"
#include "lib/surface_render.hpp"

namespace volcano {
namespace impl {

inline auto MakeApplicationInfo() {
  return ::VkApplicationInfo{
      .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,  //
      .apiVersion = VK_API_VERSION_1_3              //
  };
}
inline auto MakeInstanceCreateInfo() {
  return ::VkInstanceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO  //
  };
}
inline auto MakeDeviceCreateInfo() {
  return ::VkDeviceCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO  //
  };
}
inline auto MakeDeviceQueueCreateInfo() {
  return ::VkDeviceQueueCreateInfo{
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO  //
  };
}
inline auto MakeBufferCreateInfo() {
  return ::VkBufferCreateInfo{
      .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO  //
  };
}
inline auto MakeCommandPoolCreateInfo() {
  return ::VkCommandPoolCreateInfo{
      .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO  //
  };
}
inline auto MakeMemoryAllocateInfo() {
  return ::VkMemoryAllocateInfo{
      .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO  //
  };
}
inline auto MakeRenderPassCreateInfo() {
  return ::VkRenderPassCreateInfo{
      .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO  //
  };
}
inline auto MakePipelineLayoutCreateInfo() {
  return ::VkPipelineLayoutCreateInfo{
      .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO  //
  };
}
inline auto MakeShaderModuleCreateInfo() {
  return ::VkShaderModuleCreateInfo{
      .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO  //
  };
}
inline auto MakeSwapchainCreateInfo() {
  return ::VkSwapchainCreateInfoKHR{
      .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR  //
  };
}
inline auto MakeDebugMessengerCreateInfo() {
  return ::VkDebugUtilsMessengerCreateInfoEXT{
      .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT  //
  };
}

inline bool HasAnyFlags(::VkFlags flags, ::VkFlags query) {
  return (flags & query);
}

inline bool HasAllFlags(::VkFlags flags, ::VkFlags query) {
  return (flags & query) == query;
}

template <typename EnumerationType>
inline EnumerationType FindFirstFlag(::VkFlags flags,
                                     std::vector<EnumerationType> query,
                                     EnumerationType otherwise) {
  auto iter = std::find_if(query.begin(), query.end(),
                           [flags](auto _) { return flags & _; });
  return iter != query.end() ? *iter : otherwise;
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

inline bool HasLayerProperty(                              //
    const std::vector<::VkLayerProperties>& properties,    //
    std::string_view layer_name) {                         //
  return std::any_of(                                      //
      properties.begin(), properties.end(),                //
      [layer_name](const ::VkLayerProperties& property) {  //
        return static_cast<std::string_view>(property.layerName) == layer_name;
      });
}

inline bool HasExtensionProperty(                                  //
    const std::vector<::VkExtensionProperties>& properties,        //
    std::string_view extension_name) {                             //
  return std::any_of(                                              //
      properties.begin(), properties.end(),                        //
      [extension_name](const ::VkExtensionProperties& property) {  //
        return static_cast<std::string_view>(property.extensionName) ==
               extension_name;
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

inline std::string_view ConvertToString(
    ::VkDebugUtilsMessageSeverityFlagBitsEXT _) {
  switch (_) {
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
      return "VERB";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
      return "INFO";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
      return "WARN";
    case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
      return "ERRO";
    default:
      break;
  }
  CHECK_UNREACHABLE();
  return {};
}

inline std::string_view ConvertToString(::VkPhysicalDeviceType _) {
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
    default:
      break;
  }
  CHECK_UNREACHABLE();
  return {};
}

inline std::string_view ConvertToString(::VkQueueFlagBits _) {
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
    default:
      break;
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

inline std::string_view ConvertToString(::VkFormat _) {
  switch (_) {
    case VK_FORMAT_UNDEFINED:
      return "VK_FORMAT_UNDEFINED";
    case VK_FORMAT_R4G4_UNORM_PACK8:
      return "VK_FORMAT_R4G4_UNORM_PACK8";
    case VK_FORMAT_R4G4B4A4_UNORM_PACK16:
      return "VK_FORMAT_R4G4B4A4_UNORM_PACK16";
    case VK_FORMAT_B4G4R4A4_UNORM_PACK16:
      return "VK_FORMAT_B4G4R4A4_UNORM_PACK16";
    case VK_FORMAT_R5G6B5_UNORM_PACK16:
      return "VK_FORMAT_R5G6B5_UNORM_PACK16";
    case VK_FORMAT_B5G6R5_UNORM_PACK16:
      return "VK_FORMAT_B5G6R5_UNORM_PACK16";
    case VK_FORMAT_R5G5B5A1_UNORM_PACK16:
      return "VK_FORMAT_R5G5B5A1_UNORM_PACK16";
    case VK_FORMAT_B5G5R5A1_UNORM_PACK16:
      return "VK_FORMAT_B5G5R5A1_UNORM_PACK16";
    case VK_FORMAT_A1R5G5B5_UNORM_PACK16:
      return "VK_FORMAT_A1R5G5B5_UNORM_PACK16";
    case VK_FORMAT_R8_UNORM:
      return "VK_FORMAT_R8_UNORM";
    case VK_FORMAT_R8_SNORM:
      return "VK_FORMAT_R8_SNORM";
    case VK_FORMAT_R8_USCALED:
      return "VK_FORMAT_R8_USCALED";
    case VK_FORMAT_R8_SSCALED:
      return "VK_FORMAT_R8_SSCALED";
    case VK_FORMAT_R8_UINT:
      return "VK_FORMAT_R8_UINT";
    case VK_FORMAT_R8_SINT:
      return "VK_FORMAT_R8_SINT";
    case VK_FORMAT_R8_SRGB:
      return "VK_FORMAT_R8_SRGB";
    case VK_FORMAT_R8G8_UNORM:
      return "VK_FORMAT_R8G8_UNORM";
    case VK_FORMAT_R8G8_SNORM:
      return "VK_FORMAT_R8G8_SNORM";
    case VK_FORMAT_R8G8_USCALED:
      return "VK_FORMAT_R8G8_USCALED";
    case VK_FORMAT_R8G8_SSCALED:
      return "VK_FORMAT_R8G8_SSCALED";
    case VK_FORMAT_R8G8_UINT:
      return "VK_FORMAT_R8G8_UINT";
    case VK_FORMAT_R8G8_SINT:
      return "VK_FORMAT_R8G8_SINT";
    case VK_FORMAT_R8G8_SRGB:
      return "VK_FORMAT_R8G8_SRGB";
    case VK_FORMAT_R8G8B8_UNORM:
      return "VK_FORMAT_R8G8B8_UNORM";
    case VK_FORMAT_R8G8B8_SNORM:
      return "VK_FORMAT_R8G8B8_SNORM";
    case VK_FORMAT_R8G8B8_USCALED:
      return "VK_FORMAT_R8G8B8_USCALED";
    case VK_FORMAT_R8G8B8_SSCALED:
      return "VK_FORMAT_R8G8B8_SSCALED";
    case VK_FORMAT_R8G8B8_UINT:
      return "VK_FORMAT_R8G8B8_UINT";
    case VK_FORMAT_R8G8B8_SINT:
      return "VK_FORMAT_R8G8B8_SINT";
    case VK_FORMAT_R8G8B8_SRGB:
      return "VK_FORMAT_R8G8B8_SRGB";
    case VK_FORMAT_B8G8R8_UNORM:
      return "VK_FORMAT_B8G8R8_UNORM";
    case VK_FORMAT_B8G8R8_SNORM:
      return "VK_FORMAT_B8G8R8_SNORM";
    case VK_FORMAT_B8G8R8_USCALED:
      return "VK_FORMAT_B8G8R8_USCALED";
    case VK_FORMAT_B8G8R8_SSCALED:
      return "VK_FORMAT_B8G8R8_SSCALED";
    case VK_FORMAT_B8G8R8_UINT:
      return "VK_FORMAT_B8G8R8_UINT";
    case VK_FORMAT_B8G8R8_SINT:
      return "VK_FORMAT_B8G8R8_SINT";
    case VK_FORMAT_B8G8R8_SRGB:
      return "VK_FORMAT_B8G8R8_SRGB";
    case VK_FORMAT_R8G8B8A8_UNORM:
      return "VK_FORMAT_R8G8B8A8_UNORM";
    case VK_FORMAT_R8G8B8A8_SNORM:
      return "VK_FORMAT_R8G8B8A8_SNORM";
    case VK_FORMAT_R8G8B8A8_USCALED:
      return "VK_FORMAT_R8G8B8A8_USCALED";
    case VK_FORMAT_R8G8B8A8_SSCALED:
      return "VK_FORMAT_R8G8B8A8_SSCALED";
    case VK_FORMAT_R8G8B8A8_UINT:
      return "VK_FORMAT_R8G8B8A8_UINT";
    case VK_FORMAT_R8G8B8A8_SINT:
      return "VK_FORMAT_R8G8B8A8_SINT";
    case VK_FORMAT_R8G8B8A8_SRGB:
      return "VK_FORMAT_R8G8B8A8_SRGB";
    case VK_FORMAT_B8G8R8A8_UNORM:
      return "VK_FORMAT_B8G8R8A8_UNORM";
    case VK_FORMAT_B8G8R8A8_SNORM:
      return "VK_FORMAT_B8G8R8A8_SNORM";
    case VK_FORMAT_B8G8R8A8_USCALED:
      return "VK_FORMAT_B8G8R8A8_USCALED";
    case VK_FORMAT_B8G8R8A8_SSCALED:
      return "VK_FORMAT_B8G8R8A8_SSCALED";
    case VK_FORMAT_B8G8R8A8_UINT:
      return "VK_FORMAT_B8G8R8A8_UINT";
    case VK_FORMAT_B8G8R8A8_SINT:
      return "VK_FORMAT_B8G8R8A8_SINT";
    case VK_FORMAT_B8G8R8A8_SRGB:
      return "VK_FORMAT_B8G8R8A8_SRGB";
    case VK_FORMAT_A8B8G8R8_UNORM_PACK32:
      return "VK_FORMAT_A8B8G8R8_UNORM_PACK32";
    case VK_FORMAT_A8B8G8R8_SNORM_PACK32:
      return "VK_FORMAT_A8B8G8R8_SNORM_PACK32";
    case VK_FORMAT_A8B8G8R8_USCALED_PACK32:
      return "VK_FORMAT_A8B8G8R8_USCALED_PACK32";
    case VK_FORMAT_A8B8G8R8_SSCALED_PACK32:
      return "VK_FORMAT_A8B8G8R8_SSCALED_PACK32";
    case VK_FORMAT_A8B8G8R8_UINT_PACK32:
      return "VK_FORMAT_A8B8G8R8_UINT_PACK32";
    case VK_FORMAT_A8B8G8R8_SINT_PACK32:
      return "VK_FORMAT_A8B8G8R8_SINT_PACK32";
    case VK_FORMAT_A8B8G8R8_SRGB_PACK32:
      return "VK_FORMAT_A8B8G8R8_SRGB_PACK32";
    case VK_FORMAT_A2R10G10B10_UNORM_PACK32:
      return "VK_FORMAT_A2R10G10B10_UNORM_PACK32";
    case VK_FORMAT_A2R10G10B10_SNORM_PACK32:
      return "VK_FORMAT_A2R10G10B10_SNORM_PACK32";
    case VK_FORMAT_A2R10G10B10_USCALED_PACK32:
      return "VK_FORMAT_A2R10G10B10_USCALED_PACK32";
    case VK_FORMAT_A2R10G10B10_SSCALED_PACK32:
      return "VK_FORMAT_A2R10G10B10_SSCALED_PACK32";
    case VK_FORMAT_A2R10G10B10_UINT_PACK32:
      return "VK_FORMAT_A2R10G10B10_UINT_PACK32";
    case VK_FORMAT_A2R10G10B10_SINT_PACK32:
      return "VK_FORMAT_A2R10G10B10_SINT_PACK32";
    case VK_FORMAT_A2B10G10R10_UNORM_PACK32:
      return "VK_FORMAT_A2B10G10R10_UNORM_PACK32";
    case VK_FORMAT_A2B10G10R10_SNORM_PACK32:
      return "VK_FORMAT_A2B10G10R10_SNORM_PACK32";
    case VK_FORMAT_A2B10G10R10_USCALED_PACK32:
      return "VK_FORMAT_A2B10G10R10_USCALED_PACK32";
    case VK_FORMAT_A2B10G10R10_SSCALED_PACK32:
      return "VK_FORMAT_A2B10G10R10_SSCALED_PACK32";
    case VK_FORMAT_A2B10G10R10_UINT_PACK32:
      return "VK_FORMAT_A2B10G10R10_UINT_PACK32";
    case VK_FORMAT_A2B10G10R10_SINT_PACK32:
      return "VK_FORMAT_A2B10G10R10_SINT_PACK32";
    case VK_FORMAT_R16_UNORM:
      return "VK_FORMAT_R16_UNORM";
    case VK_FORMAT_R16_SNORM:
      return "VK_FORMAT_R16_SNORM";
    case VK_FORMAT_R16_USCALED:
      return "VK_FORMAT_R16_USCALED";
    case VK_FORMAT_R16_SSCALED:
      return "VK_FORMAT_R16_SSCALED";
    case VK_FORMAT_R16_UINT:
      return "VK_FORMAT_R16_UINT";
    case VK_FORMAT_R16_SINT:
      return "VK_FORMAT_R16_SINT";
    case VK_FORMAT_R16_SFLOAT:
      return "VK_FORMAT_R16_SFLOAT";
    case VK_FORMAT_R16G16_UNORM:
      return "VK_FORMAT_R16G16_UNORM";
    case VK_FORMAT_R16G16_SNORM:
      return "VK_FORMAT_R16G16_SNORM";
    case VK_FORMAT_R16G16_USCALED:
      return "VK_FORMAT_R16G16_USCALED";
    case VK_FORMAT_R16G16_SSCALED:
      return "VK_FORMAT_R16G16_SSCALED";
    case VK_FORMAT_R16G16_UINT:
      return "VK_FORMAT_R16G16_UINT";
    case VK_FORMAT_R16G16_SINT:
      return "VK_FORMAT_R16G16_SINT";
    case VK_FORMAT_R16G16_SFLOAT:
      return "VK_FORMAT_R16G16_SFLOAT";
    case VK_FORMAT_R16G16B16_UNORM:
      return "VK_FORMAT_R16G16B16_UNORM";
    case VK_FORMAT_R16G16B16_SNORM:
      return "VK_FORMAT_R16G16B16_SNORM";
    case VK_FORMAT_R16G16B16_USCALED:
      return "VK_FORMAT_R16G16B16_USCALED";
    case VK_FORMAT_R16G16B16_SSCALED:
      return "VK_FORMAT_R16G16B16_SSCALED";
    case VK_FORMAT_R16G16B16_UINT:
      return "VK_FORMAT_R16G16B16_UINT";
    case VK_FORMAT_R16G16B16_SINT:
      return "VK_FORMAT_R16G16B16_SINT";
    case VK_FORMAT_R16G16B16_SFLOAT:
      return "VK_FORMAT_R16G16B16_SFLOAT";
    case VK_FORMAT_R16G16B16A16_UNORM:
      return "VK_FORMAT_R16G16B16A16_UNORM";
    case VK_FORMAT_R16G16B16A16_SNORM:
      return "VK_FORMAT_R16G16B16A16_SNORM";
    case VK_FORMAT_R16G16B16A16_USCALED:
      return "VK_FORMAT_R16G16B16A16_USCALED";
    case VK_FORMAT_R16G16B16A16_SSCALED:
      return "VK_FORMAT_R16G16B16A16_SSCALED";
    case VK_FORMAT_R16G16B16A16_UINT:
      return "VK_FORMAT_R16G16B16A16_UINT";
    case VK_FORMAT_R16G16B16A16_SINT:
      return "VK_FORMAT_R16G16B16A16_SINT";
    case VK_FORMAT_R16G16B16A16_SFLOAT:
      return "VK_FORMAT_R16G16B16A16_SFLOAT";
    case VK_FORMAT_R32_UINT:
      return "VK_FORMAT_R32_UINT";
    case VK_FORMAT_R32_SINT:
      return "VK_FORMAT_R32_SINT";
    case VK_FORMAT_R32_SFLOAT:
      return "VK_FORMAT_R32_SFLOAT";
    case VK_FORMAT_R32G32_UINT:
      return "VK_FORMAT_R32G32_UINT";
    case VK_FORMAT_R32G32_SINT:
      return "VK_FORMAT_R32G32_SINT";
    case VK_FORMAT_R32G32_SFLOAT:
      return "VK_FORMAT_R32G32_SFLOAT";
    case VK_FORMAT_R32G32B32_UINT:
      return "VK_FORMAT_R32G32B32_UINT";
    case VK_FORMAT_R32G32B32_SINT:
      return "VK_FORMAT_R32G32B32_SINT";
    case VK_FORMAT_R32G32B32_SFLOAT:
      return "VK_FORMAT_R32G32B32_SFLOAT";
    case VK_FORMAT_R32G32B32A32_UINT:
      return "VK_FORMAT_R32G32B32A32_UINT";
    case VK_FORMAT_R32G32B32A32_SINT:
      return "VK_FORMAT_R32G32B32A32_SINT";
    case VK_FORMAT_R32G32B32A32_SFLOAT:
      return "VK_FORMAT_R32G32B32A32_SFLOAT";
    case VK_FORMAT_R64_UINT:
      return "VK_FORMAT_R64_UINT";
    case VK_FORMAT_R64_SINT:
      return "VK_FORMAT_R64_SINT";
    case VK_FORMAT_R64_SFLOAT:
      return "VK_FORMAT_R64_SFLOAT";
    case VK_FORMAT_R64G64_UINT:
      return "VK_FORMAT_R64G64_UINT";
    case VK_FORMAT_R64G64_SINT:
      return "VK_FORMAT_R64G64_SINT";
    case VK_FORMAT_R64G64_SFLOAT:
      return "VK_FORMAT_R64G64_SFLOAT";
    case VK_FORMAT_R64G64B64_UINT:
      return "VK_FORMAT_R64G64B64_UINT";
    case VK_FORMAT_R64G64B64_SINT:
      return "VK_FORMAT_R64G64B64_SINT";
    case VK_FORMAT_R64G64B64_SFLOAT:
      return "VK_FORMAT_R64G64B64_SFLOAT";
    case VK_FORMAT_R64G64B64A64_UINT:
      return "VK_FORMAT_R64G64B64A64_UINT";
    case VK_FORMAT_R64G64B64A64_SINT:
      return "VK_FORMAT_R64G64B64A64_SINT";
    case VK_FORMAT_R64G64B64A64_SFLOAT:
      return "VK_FORMAT_R64G64B64A64_SFLOAT";
    case VK_FORMAT_B10G11R11_UFLOAT_PACK32:
      return "VK_FORMAT_B10G11R11_UFLOAT_PACK32";
    case VK_FORMAT_E5B9G9R9_UFLOAT_PACK32:
      return "VK_FORMAT_E5B9G9R9_UFLOAT_PACK32";
    case VK_FORMAT_D16_UNORM:
      return "VK_FORMAT_D16_UNORM";
    case VK_FORMAT_X8_D24_UNORM_PACK32:
      return "VK_FORMAT_X8_D24_UNORM_PACK32";
    case VK_FORMAT_D32_SFLOAT:
      return "VK_FORMAT_D32_SFLOAT";
    case VK_FORMAT_S8_UINT:
      return "VK_FORMAT_S8_UINT";
    case VK_FORMAT_D16_UNORM_S8_UINT:
      return "VK_FORMAT_D16_UNORM_S8_UINT";
    case VK_FORMAT_D24_UNORM_S8_UINT:
      return "VK_FORMAT_D24_UNORM_S8_UINT";
    case VK_FORMAT_D32_SFLOAT_S8_UINT:
      return "VK_FORMAT_D32_SFLOAT_S8_UINT";
    case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
      return "VK_FORMAT_BC1_RGB_UNORM_BLOCK";
    case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
      return "VK_FORMAT_BC1_RGB_SRGB_BLOCK";
    case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
      return "VK_FORMAT_BC1_RGBA_UNORM_BLOCK";
    case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
      return "VK_FORMAT_BC1_RGBA_SRGB_BLOCK";
    case VK_FORMAT_BC2_UNORM_BLOCK:
      return "VK_FORMAT_BC2_UNORM_BLOCK";
    case VK_FORMAT_BC2_SRGB_BLOCK:
      return "VK_FORMAT_BC2_SRGB_BLOCK";
    case VK_FORMAT_BC3_UNORM_BLOCK:
      return "VK_FORMAT_BC3_UNORM_BLOCK";
    case VK_FORMAT_BC3_SRGB_BLOCK:
      return "VK_FORMAT_BC3_SRGB_BLOCK";
    case VK_FORMAT_BC4_UNORM_BLOCK:
      return "VK_FORMAT_BC4_UNORM_BLOCK";
    case VK_FORMAT_BC4_SNORM_BLOCK:
      return "VK_FORMAT_BC4_SNORM_BLOCK";
    case VK_FORMAT_BC5_UNORM_BLOCK:
      return "VK_FORMAT_BC5_UNORM_BLOCK";
    case VK_FORMAT_BC5_SNORM_BLOCK:
      return "VK_FORMAT_BC5_SNORM_BLOCK";
    case VK_FORMAT_BC6H_UFLOAT_BLOCK:
      return "VK_FORMAT_BC6H_UFLOAT_BLOCK";
    case VK_FORMAT_BC6H_SFLOAT_BLOCK:
      return "VK_FORMAT_BC6H_SFLOAT_BLOCK";
    case VK_FORMAT_BC7_UNORM_BLOCK:
      return "VK_FORMAT_BC7_UNORM_BLOCK";
    case VK_FORMAT_BC7_SRGB_BLOCK:
      return "VK_FORMAT_BC7_SRGB_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK:
      return "VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK:
      return "VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK:
      return "VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK:
      return "VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK:
      return "VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK";
    case VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK:
      return "VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK";
    case VK_FORMAT_EAC_R11_UNORM_BLOCK:
      return "VK_FORMAT_EAC_R11_UNORM_BLOCK";
    case VK_FORMAT_EAC_R11_SNORM_BLOCK:
      return "VK_FORMAT_EAC_R11_SNORM_BLOCK";
    case VK_FORMAT_EAC_R11G11_UNORM_BLOCK:
      return "VK_FORMAT_EAC_R11G11_UNORM_BLOCK";
    case VK_FORMAT_EAC_R11G11_SNORM_BLOCK:
      return "VK_FORMAT_EAC_R11G11_SNORM_BLOCK";
    case VK_FORMAT_ASTC_4x4_UNORM_BLOCK:
      return "VK_FORMAT_ASTC_4x4_UNORM_BLOCK";
    case VK_FORMAT_ASTC_4x4_SRGB_BLOCK:
      return "VK_FORMAT_ASTC_4x4_SRGB_BLOCK";
    case VK_FORMAT_ASTC_5x4_UNORM_BLOCK:
      return "VK_FORMAT_ASTC_5x4_UNORM_BLOCK";
    case VK_FORMAT_ASTC_5x4_SRGB_BLOCK:
      return "VK_FORMAT_ASTC_5x4_SRGB_BLOCK";
    case VK_FORMAT_ASTC_5x5_UNORM_BLOCK:
      return "VK_FORMAT_ASTC_5x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_5x5_SRGB_BLOCK:
      return "VK_FORMAT_ASTC_5x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_6x5_UNORM_BLOCK:
      return "VK_FORMAT_ASTC_6x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_6x5_SRGB_BLOCK:
      return "VK_FORMAT_ASTC_6x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_6x6_UNORM_BLOCK:
      return "VK_FORMAT_ASTC_6x6_UNORM_BLOCK";
    case VK_FORMAT_ASTC_6x6_SRGB_BLOCK:
      return "VK_FORMAT_ASTC_6x6_SRGB_BLOCK";
    case VK_FORMAT_ASTC_8x5_UNORM_BLOCK:
      return "VK_FORMAT_ASTC_8x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_8x5_SRGB_BLOCK:
      return "VK_FORMAT_ASTC_8x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_8x6_UNORM_BLOCK:
      return "VK_FORMAT_ASTC_8x6_UNORM_BLOCK";
    case VK_FORMAT_ASTC_8x6_SRGB_BLOCK:
      return "VK_FORMAT_ASTC_8x6_SRGB_BLOCK";
    case VK_FORMAT_ASTC_8x8_UNORM_BLOCK:
      return "VK_FORMAT_ASTC_8x8_UNORM_BLOCK";
    case VK_FORMAT_ASTC_8x8_SRGB_BLOCK:
      return "VK_FORMAT_ASTC_8x8_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x5_UNORM_BLOCK:
      return "VK_FORMAT_ASTC_10x5_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x5_SRGB_BLOCK:
      return "VK_FORMAT_ASTC_10x5_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x6_UNORM_BLOCK:
      return "VK_FORMAT_ASTC_10x6_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x6_SRGB_BLOCK:
      return "VK_FORMAT_ASTC_10x6_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x8_UNORM_BLOCK:
      return "VK_FORMAT_ASTC_10x8_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x8_SRGB_BLOCK:
      return "VK_FORMAT_ASTC_10x8_SRGB_BLOCK";
    case VK_FORMAT_ASTC_10x10_UNORM_BLOCK:
      return "VK_FORMAT_ASTC_10x10_UNORM_BLOCK";
    case VK_FORMAT_ASTC_10x10_SRGB_BLOCK:
      return "VK_FORMAT_ASTC_10x10_SRGB_BLOCK";
    case VK_FORMAT_ASTC_12x10_UNORM_BLOCK:
      return "VK_FORMAT_ASTC_12x10_UNORM_BLOCK";
    case VK_FORMAT_ASTC_12x10_SRGB_BLOCK:
      return "VK_FORMAT_ASTC_12x10_SRGB_BLOCK";
    case VK_FORMAT_ASTC_12x12_UNORM_BLOCK:
      return "VK_FORMAT_ASTC_12x12_UNORM_BLOCK";
    case VK_FORMAT_ASTC_12x12_SRGB_BLOCK:
      return "VK_FORMAT_ASTC_12x12_SRGB_BLOCK";
    case VK_FORMAT_G8B8G8R8_422_UNORM:
      return "VK_FORMAT_G8B8G8R8_422_UNORM";
    case VK_FORMAT_B8G8R8G8_422_UNORM:
      return "VK_FORMAT_B8G8R8G8_422_UNORM";
    case VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM:
      return "VK_FORMAT_G8_B8_R8_3PLANE_420_UNORM";
    case VK_FORMAT_G8_B8R8_2PLANE_420_UNORM:
      return "VK_FORMAT_G8_B8R8_2PLANE_420_UNORM";
    case VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM:
      return "VK_FORMAT_G8_B8_R8_3PLANE_422_UNORM";
    case VK_FORMAT_G8_B8R8_2PLANE_422_UNORM:
      return "VK_FORMAT_G8_B8R8_2PLANE_422_UNORM";
    case VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM:
      return "VK_FORMAT_G8_B8_R8_3PLANE_444_UNORM";
    case VK_FORMAT_R10X6_UNORM_PACK16:
      return "VK_FORMAT_R10X6_UNORM_PACK16";
    case VK_FORMAT_R10X6G10X6_UNORM_2PACK16:
      return "VK_FORMAT_R10X6G10X6_UNORM_2PACK16";
    case VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16:
      return "VK_FORMAT_R10X6G10X6B10X6A10X6_UNORM_4PACK16";
    case VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16:
      return "VK_FORMAT_G10X6B10X6G10X6R10X6_422_UNORM_4PACK16";
    case VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16:
      return "VK_FORMAT_B10X6G10X6R10X6G10X6_422_UNORM_4PACK16";
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16:
      return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16";
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16:
      return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16";
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16:
      return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16";
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16:
      return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16";
    case VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16:
      return "VK_FORMAT_G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16";
    case VK_FORMAT_R12X4_UNORM_PACK16:
      return "VK_FORMAT_R12X4_UNORM_PACK16";
    case VK_FORMAT_R12X4G12X4_UNORM_2PACK16:
      return "VK_FORMAT_R12X4G12X4_UNORM_2PACK16";
    case VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16:
      return "VK_FORMAT_R12X4G12X4B12X4A12X4_UNORM_4PACK16";
    case VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16:
      return "VK_FORMAT_G12X4B12X4G12X4R12X4_422_UNORM_4PACK16";
    case VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16:
      return "VK_FORMAT_B12X4G12X4R12X4G12X4_422_UNORM_4PACK16";
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16:
      return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16";
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16:
      return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16";
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16:
      return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16";
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16:
      return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16";
    case VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16:
      return "VK_FORMAT_G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16";
    case VK_FORMAT_G16B16G16R16_422_UNORM:
      return "VK_FORMAT_G16B16G16R16_422_UNORM";
    case VK_FORMAT_B16G16R16G16_422_UNORM:
      return "VK_FORMAT_B16G16R16G16_422_UNORM";
    case VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM:
      return "VK_FORMAT_G16_B16_R16_3PLANE_420_UNORM";
    case VK_FORMAT_G16_B16R16_2PLANE_420_UNORM:
      return "VK_FORMAT_G16_B16R16_2PLANE_420_UNORM";
    case VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM:
      return "VK_FORMAT_G16_B16_R16_3PLANE_422_UNORM";
    case VK_FORMAT_G16_B16R16_2PLANE_422_UNORM:
      return "VK_FORMAT_G16_B16R16_2PLANE_422_UNORM";
    case VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM:
      return "VK_FORMAT_G16_B16_R16_3PLANE_444_UNORM";
    case VK_FORMAT_G8_B8R8_2PLANE_444_UNORM:
      return "VK_FORMAT_G8_B8R8_2PLANE_444_UNORM";
    case VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16:
      return "VK_FORMAT_G10X6_B10X6R10X6_2PLANE_444_UNORM_3PACK16";
    case VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16:
      return "VK_FORMAT_G12X4_B12X4R12X4_2PLANE_444_UNORM_3PACK16";
    case VK_FORMAT_G16_B16R16_2PLANE_444_UNORM:
      return "VK_FORMAT_G16_B16R16_2PLANE_444_UNORM";
    case VK_FORMAT_A4R4G4B4_UNORM_PACK16:
      return "VK_FORMAT_A4R4G4B4_UNORM_PACK16";
    case VK_FORMAT_A4B4G4R4_UNORM_PACK16:
      return "VK_FORMAT_A4B4G4R4_UNORM_PACK16";
    case VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK:
      return "VK_FORMAT_ASTC_4x4_SFLOAT_BLOCK";
    case VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK:
      return "VK_FORMAT_ASTC_5x4_SFLOAT_BLOCK";
    case VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK:
      return "VK_FORMAT_ASTC_5x5_SFLOAT_BLOCK";
    case VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK:
      return "VK_FORMAT_ASTC_6x5_SFLOAT_BLOCK";
    case VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK:
      return "VK_FORMAT_ASTC_6x6_SFLOAT_BLOCK";
    case VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK:
      return "VK_FORMAT_ASTC_8x5_SFLOAT_BLOCK";
    case VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK:
      return "VK_FORMAT_ASTC_8x6_SFLOAT_BLOCK";
    case VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK:
      return "VK_FORMAT_ASTC_8x8_SFLOAT_BLOCK";
    case VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK:
      return "VK_FORMAT_ASTC_10x5_SFLOAT_BLOCK";
    case VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK:
      return "VK_FORMAT_ASTC_10x6_SFLOAT_BLOCK";
    case VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK:
      return "VK_FORMAT_ASTC_10x8_SFLOAT_BLOCK";
    case VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK:
      return "VK_FORMAT_ASTC_10x10_SFLOAT_BLOCK";
    case VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK:
      return "VK_FORMAT_ASTC_12x10_SFLOAT_BLOCK";
    case VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK:
      return "VK_FORMAT_ASTC_12x12_SFLOAT_BLOCK";
    default:
      break;
  }
  CHECK_UNREACHABLE();
  return {};
}

inline std::string_view ConvertToString(::VkPresentModeKHR _) {
  switch (_) {
    case VK_PRESENT_MODE_IMMEDIATE_KHR:
      return "VK_PRESENT_MODE_IMMEDIATE_KHR";
    case VK_PRESENT_MODE_MAILBOX_KHR:
      return "VK_PRESENT_MODE_MAILBOX_KHR";
    case VK_PRESENT_MODE_FIFO_KHR:
      return "VK_PRESENT_MODE_FIFO_KHR";
    case VK_PRESENT_MODE_FIFO_RELAXED_KHR:
      return "VK_PRESENT_MODE_FIFO_RELAXED_KHR";
    default:
      break;
  }
  CHECK_UNREACHABLE();
  return {};
}

constexpr const char* VALIDATION_LAYER_NAME = "VK_LAYER_KHRONOS_validation";
constexpr const char* SWAPCHAIN_EXTENSION_NAME =
    VK_KHR_SWAPCHAIN_EXTENSION_NAME;
constexpr const char* DEBUG_EXTENSION_NAME = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
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
class Instance;
class Device;

//------------------------------------------------------------------------------
class Queue final {
 public:
  DECLARE_COPY_DELETE(Queue);
  DECLARE_MOVE_DEFAULT(Queue);

  Queue() = delete;
  ~Queue() = default;

  std::uint32_t FamilyIndex() const { return queue_family_index_; }

 private:
  ::VkQueue queue_ = VK_NULL_HANDLE;
  std::uint32_t queue_family_index_ = std::numeric_limits<std::uint32_t>::max();

  friend class Device;

  explicit Queue(::VkDevice device,                 //
                 std::uint32_t queue_family_index,  //
                 std::uint32_t queue_index)
      : queue_family_index_{queue_family_index} {
    CHECK_PRECONDITION(device != VK_NULL_HANDLE);
    ::vkGetDeviceQueue(device, queue_family_index, queue_index,
                       std::addressof(queue_));
  }
};

//------------------------------------------------------------------------------
class Buffer final {
 public:
  DECLARE_COPY_DELETE(Buffer);
  DECLARE_MOVE_DEFAULT(Buffer);

  Buffer() = delete;
  ~Buffer() {
    if (buffer_ != VK_NULL_HANDLE) {
      CHECK_INVARIANT(device_ != VK_NULL_HANDLE);
      ::vkDestroyBuffer(device_, buffer_, impl::ALLOCATOR);
    }
  }

 private:
  friend class Device;

  explicit Buffer(::VkDevice device,          //
                  ::VkDeviceSize byte_count,  //
                  ::VkBufferUsageFlags buffer_usage)
      : device_{device} {
    buffer_info_.size = byte_count;
    buffer_info_.usage = buffer_usage;
    buffer_info_.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    ::VkResult result =
        ::vkCreateBuffer(device_, std::addressof(buffer_info_), impl::ALLOCATOR,
                         std::addressof(buffer_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);

    ::vkGetBufferMemoryRequirements(device_, buffer_,
                                    std::addressof(memory_requirements_));
  }

  ::VkDevice device_ = VK_NULL_HANDLE;
  ::VkBuffer buffer_ = VK_NULL_HANDLE;
  ::VkBufferCreateInfo buffer_info_ = impl::MakeBufferCreateInfo();
  ::VkMemoryRequirements memory_requirements_;
};

//------------------------------------------------------------------------------
class DeviceMemory final {
 public:
  DECLARE_COPY_DELETE(DeviceMemory);
  DECLARE_MOVE_DEFAULT(DeviceMemory);

  DeviceMemory() = delete;
  ~DeviceMemory() {
    if (memory_ != VK_NULL_HANDLE) {
      CHECK_INVARIANT(device_ != VK_NULL_HANDLE);
      ::vkFreeMemory(device_, memory_, impl::ALLOCATOR);
    }
  }

  void CopyInitialize(std::span<const std::byte> data) {
    CHECK_PRECONDITION(data.size() <= memory_info_.allocationSize);
    CHECK_PRECONDITION(host_bytes_);

    std::copy(data.begin(), data.end(), host_bytes_);
    ::vkUnmapMemory(device_, memory_);

    host_bytes_ = nullptr;
  }

 private:
  friend class Device;

  explicit DeviceMemory(::VkDevice device,                    //
                        ::VkDeviceSize required_byte_offset,  //
                        ::VkDeviceSize required_byte_count,   //
                        std::uint32_t memory_type_index,      //
                        ::VkBuffer target_buffer)
      : device_{device} {
    memory_info_.allocationSize = required_byte_count;
    memory_info_.memoryTypeIndex = memory_type_index;

    ::VkResult result =
        ::vkAllocateMemory(device_, std::addressof(memory_info_),
                           impl::ALLOCATOR, std::addressof(memory_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);

    result = ::vkBindBufferMemory(device_, target_buffer, memory_,
                                  required_byte_offset);
    CHECK_POSTCONDITION(result == VK_SUCCESS);

    void* host_pointer = nullptr;
    ::VkMemoryMapFlags flags = 0;
    result = ::vkMapMemory(device_, memory_, required_byte_offset,
                           VK_WHOLE_SIZE, flags, std::addressof(host_pointer));
    CHECK_POSTCONDITION(result == VK_SUCCESS);

    host_bytes_ = reinterpret_cast<std::byte*>(host_pointer);
  }

  std::byte* host_bytes_ = nullptr;
  ::VkDevice device_ = VK_NULL_HANDLE;
  ::VkDeviceMemory memory_ = VK_NULL_HANDLE;
  ::VkMemoryAllocateInfo memory_info_ = impl::MakeMemoryAllocateInfo();
};

//------------------------------------------------------------------------------
class CommandPool final {
 public:
  DECLARE_COPY_DELETE(CommandPool);
  DECLARE_MOVE_DEFAULT(CommandPool);

  CommandPool() = delete;
  ~CommandPool() {
    if (command_pool_ != VK_NULL_HANDLE) {
      CHECK_INVARIANT(device_ != VK_NULL_HANDLE);
      ::vkDestroyCommandPool(device_, command_pool_, impl::ALLOCATOR);
    }
  }

 private:
  friend class Device;

  explicit CommandPool(::VkDevice device, std::uint32_t queue_family_index)
      : device_{device} {
    command_pool_info_.queueFamilyIndex = queue_family_index;

    ::VkResult result =
        ::vkCreateCommandPool(device_, std::addressof(command_pool_info_),
                              impl::ALLOCATOR, std::addressof(command_pool_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

  ::VkDevice device_ = VK_NULL_HANDLE;
  ::VkCommandPool command_pool_ = VK_NULL_HANDLE;
  ::VkCommandPoolCreateInfo command_pool_info_ =
      impl::MakeCommandPoolCreateInfo();
};

//------------------------------------------------------------------------------
class RenderPass final {
 public:
  DECLARE_COPY_DELETE(RenderPass);
  DECLARE_MOVE_DEFAULT(RenderPass);

  RenderPass() = delete;
  ~RenderPass() {
    if (render_pass_ != VK_NULL_HANDLE) {
      CHECK_INVARIANT(device_ != VK_NULL_HANDLE);
      ::vkDestroyRenderPass(device_, render_pass_, impl::ALLOCATOR);
    }
  }

 private:
  friend class Device;

  explicit RenderPass(::VkDevice device, ::VkFormat format) : device_{device} {
    CHECK_PRECONDITION(device_ != VK_NULL_HANDLE);

    static std::array<::VkAttachmentDescription, 1>  //
        color_attachment{::VkAttachmentDescription{
            .samples = VK_SAMPLE_COUNT_1_BIT,                    //
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,               //
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,             //
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,    //
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,  //
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,          //
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR       //
        }};

    static const std::array<const ::VkAttachmentReference, 1>  //
        color_reference{::VkAttachmentReference{
            .attachment = 0,                                    //
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL  //
        }};

    static const std::array<const ::VkSubpassDescription, 1>  //
        subpass_description{::VkSubpassDescription{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,  //
            .inputAttachmentCount = 0,                             //
            .pInputAttachments = nullptr,                          //
            .colorAttachmentCount = color_reference.size(),        //
            .pColorAttachments = color_reference.data(),           //
            .pResolveAttachments = nullptr,                        //
            .pDepthStencilAttachment = nullptr,                    //
            .preserveAttachmentCount = 0,                          //
            .pPreserveAttachments = nullptr                        //
        }};

    static const ::VkSubpassDependency src_subpass_dependency{
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT};

    static const ::VkSubpassDependency dst_subpass_dependency{
        .srcSubpass = 0,
        .dstSubpass = VK_SUBPASS_EXTERNAL,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dstAccessMask = 0,
        .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT};

    static const std::array<const ::VkSubpassDependency, 2>
        subpass_dependencies{src_subpass_dependency, dst_subpass_dependency};

    color_attachment.front().format = format;
    render_pass_info_.attachmentCount = color_attachment.size();
    render_pass_info_.pAttachments = color_attachment.data();
    render_pass_info_.subpassCount = subpass_description.size();
    render_pass_info_.pSubpasses = subpass_description.data();
    render_pass_info_.dependencyCount = subpass_dependencies.size();
    render_pass_info_.pDependencies = subpass_dependencies.data();

    ::VkResult result =
        ::vkCreateRenderPass(device_, std::addressof(render_pass_info_),
                             impl::ALLOCATOR, std::addressof(render_pass_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

  ::VkDevice device_ = VK_NULL_HANDLE;
  ::VkRenderPass render_pass_ = VK_NULL_HANDLE;
  ::VkRenderPassCreateInfo render_pass_info_ = impl::MakeRenderPassCreateInfo();
};

//------------------------------------------------------------------------------
class PipelineLayout final {
 public:
  DECLARE_COPY_DELETE(PipelineLayout);
  DECLARE_MOVE_DEFAULT(PipelineLayout);

  PipelineLayout() = delete;
  ~PipelineLayout() {
    if (pipeline_layout_ != VK_NULL_HANDLE) {
      CHECK_INVARIANT(device_ != VK_NULL_HANDLE);
      ::vkDestroyPipelineLayout(device_, pipeline_layout_, impl::ALLOCATOR);
    }
  }

 private:
  friend class Device;

  explicit PipelineLayout(::VkDevice device) : device_{device} {
    ::VkResult result = ::vkCreatePipelineLayout(
        device_, std::addressof(pipeline_layout_info_), impl::ALLOCATOR,
        std::addressof(pipeline_layout_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

  ::VkDevice device_ = VK_NULL_HANDLE;
  ::VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
  ::VkPipelineLayoutCreateInfo pipeline_layout_info_ =
      impl::MakePipelineLayoutCreateInfo();
};

//------------------------------------------------------------------------------
class ShaderModule final {
 public:
  DECLARE_COPY_DELETE(ShaderModule);
  DECLARE_MOVE_DEFAULT(ShaderModule);

  ShaderModule() = delete;
  ~ShaderModule() {
    if (shader_module_ != VK_NULL_HANDLE) {
      CHECK_INVARIANT(device_ != VK_NULL_HANDLE);
      ::vkDestroyShaderModule(device_, shader_module_, impl::ALLOCATOR);
    }
  }

 private:
  friend class Device;

  explicit ShaderModule(::VkDevice device,
                        const std::vector<std::uint32_t>& shader_spirv_bin)
      : device_{device} {
    shader_module_info_.pCode = shader_spirv_bin.data();
    shader_module_info_.codeSize =  // Byte count.
        shader_spirv_bin.size() * sizeof(std::uint32_t);

    ::VkResult result =
        ::vkCreateShaderModule(device_, std::addressof(shader_module_info_),
                               impl::ALLOCATOR, std::addressof(shader_module_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

  ::VkDevice device_ = VK_NULL_HANDLE;
  ::VkShaderModule shader_module_ = VK_NULL_HANDLE;
  ::VkShaderModuleCreateInfo shader_module_info_ =
      impl::MakeShaderModuleCreateInfo();
};

//------------------------------------------------------------------------------
class Swapchain final {
 public:
  DECLARE_COPY_DELETE(Swapchain);
  DECLARE_MOVE_DEFAULT(Swapchain);

  Swapchain() = delete;
  ~Swapchain() {
    if (swapchain_ != VK_NULL_HANDLE) {
      CHECK_INVARIANT(device_ != VK_NULL_HANDLE);
      ::vkDestroySwapchainKHR(device_, swapchain_, impl::ALLOCATOR);
    }
  }

 private:
  friend class Device;

  explicit Swapchain(::VkDevice device,                                       //
                     std::vector<std::uint32_t> queue_families,               //
                     ::VkSurfaceKHR surface,                                  //
                     const ::VkSurfaceCapabilitiesKHR& surface_capabilities,  //
                     const ::VkSurfaceFormatKHR& surface_format,
                     ::VkPresentModeKHR surface_present_mode,
                     ::VkSwapchainKHR previous_swapchain)
      : device_{device},
        queue_families_{std::move(queue_families)},
        surface_{surface},
        surface_capabilities_{surface_capabilities},
        surface_format_{surface_format} {
    swapchain_info_.surface = surface_;
    swapchain_info_.minImageCount = surface_capabilities_.minImageCount + 1;
    swapchain_info_.imageFormat = surface_format_.format;
    swapchain_info_.imageColorSpace = surface_format_.colorSpace;
    swapchain_info_.imageExtent = surface_capabilities_.currentExtent;
    swapchain_info_.imageArrayLayers = 1;  // Non-stereoscopic.
    swapchain_info_.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchain_info_.imageSharingMode = queue_families_.size() > 1
                                           ? VK_SHARING_MODE_CONCURRENT
                                           : VK_SHARING_MODE_EXCLUSIVE;
    swapchain_info_.queueFamilyIndexCount = queue_families_.size();
    swapchain_info_.pQueueFamilyIndices = queue_families_.data();
    swapchain_info_.preTransform = surface_capabilities_.currentTransform;
    swapchain_info_.compositeAlpha =
        impl::FindFirstFlag(surface_capabilities_.supportedCompositeAlpha,
                            {VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                             VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                             VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
                             VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR},
                            static_cast<::VkCompositeAlphaFlagBitsKHR>(-1));
    CHECK_INVARIANT(swapchain_info_.compositeAlpha != -1);

    swapchain_info_.presentMode = surface_present_mode;
    swapchain_info_.clipped = VK_TRUE;
    swapchain_info_.oldSwapchain = previous_swapchain;

    ::VkResult result =
        ::vkCreateSwapchainKHR(device_, std::addressof(swapchain_info_),
                               impl::ALLOCATOR, std::addressof(swapchain_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

  ::VkDevice device_ = VK_NULL_HANDLE;
  ::VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
  std::vector<std::uint32_t> queue_families_;

  ::VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  ::VkSurfaceCapabilitiesKHR surface_capabilities_;
  ::VkSurfaceFormatKHR surface_format_;
  ::VkSwapchainCreateInfoKHR swapchain_info_ = impl::MakeSwapchainCreateInfo();
};

//------------------------------------------------------------------------------
class Device final {
 public:
  DECLARE_COPY_DELETE(Device);
  DECLARE_MOVE_DEFAULT(Device);

  Device() = delete;
  ~Device() {
    if (device_ != VK_NULL_HANDLE) {
      ::vkDestroyDevice(device_, impl::ALLOCATOR);
    }
  }

  Queue CreateQueue() {
    CHECK_PRECONDITION(queue_families_.size() == 1);
    return Queue{device_, queue_families_.front(), 0u};
  }

  Buffer CreateBuffer(::VkDeviceSize requested_byte_count,
                      ::VkBufferUsageFlags requested_buffer_usage) {
    return Buffer{
        device_,
        requested_byte_count,
        requested_buffer_usage,
    };
  }

  DeviceMemory AllocateDeviceMemory(
      const Buffer& buffer, ::VkMemoryPropertyFlags required_memory_flags) {
    bool found = false;

    std::uint32_t memory_type_index = 0;
    for (; memory_type_index < (sizeof(std::uint32_t) * 8) &&
           memory_type_index < phys_device_memory_properties_.memoryTypeCount;
         ++memory_type_index) {
      if ((buffer.memory_requirements_.memoryTypeBits &
           (1u << memory_type_index)) &&
          impl::HasAllFlags(
              phys_device_memory_properties_.memoryTypes[memory_type_index]
                  .propertyFlags,
              required_memory_flags)) {
        found = true;
        break;
      }
    }
    CHECK_POSTCONDITION(found);

    ::VkDeviceSize byte_offset = 0;
    return DeviceMemory{device_, byte_offset, buffer.memory_requirements_.size,
                        memory_type_index, buffer.buffer_};
  }

  CommandPool CreateCommandPool(std::uint32_t queue_family_index) {
    return CommandPool{device_, queue_family_index};
  }

  RenderPass CreateRenderPass(::VkFormat requested) {
    CHECK_PRECONDITION(std::any_of(surface_formats_.begin(),
                                   surface_formats_.end(),
                                   [requested](::VkSurfaceFormatKHR supported) {
                                     return supported.format == requested;
                                   }));
    return RenderPass{device_, requested};
  }

  std::unique_ptr<SurfaceRenderer> CreateSurfaceRenderer() {
    return std::make_unique<SurfaceRenderer>(surface_,               //
                                             surface_capabilities_,  //
                                             surface_formats_);
  }

  PipelineLayout CreatePipelineLayout() {
    return PipelineLayout{device_};  //
  }

  ShaderModule CreateShaderModule(
      const std::vector<std::uint32_t>& shader_spirv_bin) {
    return ShaderModule{device_, shader_spirv_bin};
  }

  Swapchain CreateSwapchain(
      ::VkFormat requested_format,              //
      ::VkPresentModeKHR surface_present_mode,  //
      ::VkSwapchainKHR previous_swapchain = VK_NULL_HANDLE) {
    auto surface_format_iter =
        std::find_if(surface_formats_.begin(), surface_formats_.end(),
                     [requested_format](::VkSurfaceFormatKHR supported) {
                       return supported.format == requested_format;
                     });
    CHECK_PRECONDITION(surface_format_iter != surface_formats_.end());
    CHECK_PRECONDITION(std::find(surface_present_modes_.begin(),  //
                                 surface_present_modes_.end(),    //
                                 surface_present_mode) !=
                       surface_present_modes_.end());

    return Swapchain{device_,                //
                     queue_families_,        //
                     surface_,               //
                     surface_capabilities_,  //
                     *surface_format_iter,   //
                     surface_present_mode,   //
                     previous_swapchain};
  }

 private:
  friend class Instance;

  explicit Device(
      ::VkSurfaceKHR surface,                                       //
      ::VkPhysicalDevice phys_device,                               //
      const ::VkPhysicalDeviceFeatures& features,                   //
      const ::VkPhysicalDeviceMemoryProperties& memory_properties,  //
      std::vector<const char*> device_extensions,                   //
      std::vector<std::uint32_t> queue_families)
      : surface_{surface},
        phys_device_features_{features},
        phys_device_memory_properties_{memory_properties},
        device_extensions_{std::move(device_extensions)},
        queue_families_{std::move(queue_families)} {
    static const std::array<float, 1> queue_priority{1.0f};  // [0.0, 1.0]

    for (auto queue_family_i : queue_families_) {
      device_queue_infos_.push_back(impl::MakeDeviceQueueCreateInfo());
      device_queue_infos_.back().queueFamilyIndex = queue_family_i;
      device_queue_infos_.back().queueCount = queue_priority.size();
      device_queue_infos_.back().pQueuePriorities = queue_priority.data();
    }

    device_info_.queueCreateInfoCount = device_queue_infos_.size();
    device_info_.pQueueCreateInfos = device_queue_infos_.data();
    device_info_.enabledExtensionCount = device_extensions_.size();
    device_info_.ppEnabledExtensionNames = device_extensions_.data();
    device_info_.pEnabledFeatures = std::addressof(phys_device_features_);

    ::VkResult result =
        ::vkCreateDevice(phys_device, std::addressof(device_info_),
                         impl::ALLOCATOR, std::addressof(device_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);

    result = ::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        phys_device, surface_, std::addressof(surface_capabilities_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);

    impl::MaybeEnumerateProperties(
        std::bind_front(::vkGetPhysicalDeviceSurfaceFormatsKHR, phys_device,
                        surface_),
        InOut(surface_formats_));

    std::print("Surface Formats: \n");
    for (auto&& surface_format : surface_formats_) {
      std::print(" :: {}\n", impl::ConvertToString(surface_format.format));
    }

    impl::MaybeEnumerateProperties(
        std::bind_front(::vkGetPhysicalDeviceSurfacePresentModesKHR,
                        phys_device, surface_),
        InOut(surface_present_modes_));

    std::print("Surface Present Modes: \n");
    for (auto&& surface_present_mode : surface_present_modes_) {
      std::print(" .. {}\n", impl::ConvertToString(surface_present_mode));
    }
  }

  ::VkDevice device_ = VK_NULL_HANDLE;
  ::VkDeviceCreateInfo device_info_ = impl::MakeDeviceCreateInfo();

  ::VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  ::VkSurfaceCapabilitiesKHR surface_capabilities_;
  std::vector<::VkSurfaceFormatKHR> surface_formats_;
  std::vector<::VkPresentModeKHR> surface_present_modes_;

  ::VkPhysicalDeviceFeatures phys_device_features_;
  ::VkPhysicalDeviceMemoryProperties phys_device_memory_properties_;

  std::vector<::VkDeviceQueueCreateInfo> device_queue_infos_;
  std::vector<const char*> device_extensions_;
  std::vector<std::uint32_t> queue_families_;
};

//------------------------------------------------------------------------------
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

  ::VkInstance Handle() const { return instance_; }

  Device CreateDevice(::VkSurfaceKHR surface) {
    CHECK_PRECONDITION(surface != VK_NULL_HANDLE);

    std::vector<FindQueueFamilyResult> selected_result = SelectQueueFamilyIf(
        [surface](
            ::VkPhysicalDevice phys_device,
            const ::VkPhysicalDeviceProperties& phys_device_property,
            std::uint32_t queue_family_i,
            const ::VkQueueFamilyProperties& queue_family_properties) -> bool {
          if ((phys_device_property.deviceType ==
               VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) &&
              (queue_family_properties.queueFlags & VK_QUEUE_GRAPHICS_BIT)) {
            ::VkBool32 is_supported = VK_FALSE;
            ::VkResult selected_result = ::vkGetPhysicalDeviceSurfaceSupportKHR(
                phys_device, queue_family_i, surface,
                std::addressof(is_supported));
            CHECK_POSTCONDITION(selected_result == VK_SUCCESS);
            return is_supported;
          }
          return false;
        });
    CHECK_POSTCONDITION(selected_result.size());
    CHECK_POSTCONDITION(selected_result.front().phys_device != VK_NULL_HANDLE);

    ::VkPhysicalDevice selected_phys_device =
        selected_result.front().phys_device;
    std::uint32_t selected_queue_family_index =
        selected_result.front().queue_family_index;

    return Device{surface,
                  selected_phys_device,
                  phys_device_features_[selected_phys_device],
                  phys_device_memory_properties_[selected_phys_device],
                  {impl::SWAPCHAIN_EXTENSION_NAME},
                  {{selected_queue_family_index}}};
  }

 private:
  friend class Application;

  explicit Instance(const ::VkApplicationInfo* app_info,  //
                    std::vector<const char*> layers,      //
                    std::vector<const char*> extensions,  //
                    DebugLevel debug_level)
      : instance_layers_{std::move(layers)},
        instance_extensions_{std::move(extensions)} {
    instance_info_.pApplicationInfo = app_info;

    if (debug_level != DebugLevel::NONE &&
        impl::HasStringName(instance_extensions_, impl::DEBUG_EXTENSION_NAME)) {
      instance_info_.pNext = std::addressof(debug_messenger_info_);

      debug_messenger_info_.messageSeverity =
          ConvertToDebugSeverity(debug_level);
      debug_messenger_info_.messageType =
          VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      debug_messenger_info_.pfnUserCallback = DebugMessengerCallback;
    }

    instance_info_.enabledLayerCount = instance_layers_.size();
    instance_info_.ppEnabledLayerNames = instance_layers_.data();
    instance_info_.enabledExtensionCount = instance_extensions_.size();
    instance_info_.ppEnabledExtensionNames = instance_extensions_.data();

    std::print("Requested Layers: \n");
    for (auto&& layer : instance_layers_) {
      std::print(" == {}\n", layer);
    }

    std::print("Requested Extensions: \n");
    for (auto&& extension : instance_extensions_) {
      std::print(" -- {}\n", extension);
    }

    ::VkResult result =
        ::vkCreateInstance(std::addressof(instance_info_), impl::ALLOCATOR,
                           std::addressof(instance_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);

    if (debug_level != DebugLevel::NONE &&
        impl::HasStringName(instance_extensions_, impl::DEBUG_EXTENSION_NAME)) {
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
    }

    impl::MaybeEnumerateProperties(
        std::bind_front(::vkEnumeratePhysicalDevices, instance_),
        InOut(phys_devices_));

    std::print("Physical Devices: \n");
    for (auto&& phys_device : phys_devices_) {
      ::vkGetPhysicalDeviceProperties(
          phys_device, std::addressof(phys_device_properties_[phys_device]));
      std::print(" ** {} [{}]\n",
                 phys_device_properties_[phys_device].deviceName,
                 impl::ConvertToString(
                     phys_device_properties_[phys_device].deviceType));

      ::vkGetPhysicalDeviceMemoryProperties(
          phys_device,
          std::addressof(phys_device_memory_properties_[phys_device]));

      ::vkGetPhysicalDeviceFeatures(
          phys_device, std::addressof(phys_device_features_[phys_device]));

      impl::MaybeEnumerateProperties(
          std::bind_front(::vkGetPhysicalDeviceQueueFamilyProperties,
                          phys_device),
          InOut(phys_device_queue_family_properties_[phys_device]));

      std::print("Queue Family Flags: \n");
      for (auto&& property :
           phys_device_queue_family_properties_[phys_device]) {
        std::print(" .. [{}] {}\n", property.queueCount,
                   impl::ConvertToString(property.queueFlags));
      }

      impl::MaybeEnumerateProperties(
          std::bind_front(::vkEnumerateDeviceExtensionProperties, phys_device,
                          nullptr),
          InOut(supported_device_extension_properties_[phys_device]));

      std::print("Supported Device Extensions: \n");
      for (auto&& property :
           supported_device_extension_properties_[phys_device]) {
        std::print(" -- {}\n", property.extensionName);
      }

      CHECK_INVARIANT(impl::HasExtensionProperty(
          supported_device_extension_properties_[phys_device],
          impl::SWAPCHAIN_EXTENSION_NAME));
    }
  }

  struct FindQueueFamilyResult final {
    ::VkPhysicalDevice phys_device = VK_NULL_HANDLE;
    std::uint32_t queue_family_index = 0;
  };

  template <typename PredicateType>
  std::vector<FindQueueFamilyResult> SelectQueueFamilyIf(
      PredicateType&& predicate) {
    std::vector<FindQueueFamilyResult> result;

    for (auto&& phys_device : phys_devices_) {
      auto&& phys_device_property = phys_device_properties_[phys_device];
      auto&& queue_family_properties =
          phys_device_queue_family_properties_[phys_device];

      for (std::uint32_t queue_family_i = 0;                 //
           queue_family_i < queue_family_properties.size();  //
           ++queue_family_i) {
        auto&& queue_family_property = queue_family_properties[queue_family_i];

        if (predicate(phys_device,           //
                      phys_device_property,  //
                      queue_family_i,        //
                      queue_family_property)) {
          result.push_back({
              .phys_device = phys_device,           //
              .queue_family_index = queue_family_i  //
          });
        }
      }
    }

    return result;
  }

  std::uint32_t SelectMemoryFromRequirements() {
    std::uint32_t device_memory_property_index = 0;

    return device_memory_property_index;
  }

  ::VkInstance instance_ = VK_NULL_HANDLE;
  ::VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;

  ::PFN_vkSubmitDebugUtilsMessageEXT submit_debug_message_ = nullptr;
  ::PFN_vkCreateDebugUtilsMessengerEXT create_debug_messenger_ = nullptr;
  ::PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_messenger_ = nullptr;

  ::VkInstanceCreateInfo instance_info_ = impl::MakeInstanceCreateInfo();
  ::VkDebugUtilsMessengerCreateInfoEXT debug_messenger_info_ =
      impl::MakeDebugMessengerCreateInfo();

  std::vector<const char*> instance_layers_;
  std::vector<const char*> instance_extensions_;
  std::vector<::VkPhysicalDevice> phys_devices_;
  std::map<::VkPhysicalDevice, ::VkPhysicalDeviceProperties>
      phys_device_properties_;
  std::map<::VkPhysicalDevice, ::VkPhysicalDeviceMemoryProperties>
      phys_device_memory_properties_;
  std::map<::VkPhysicalDevice, ::VkPhysicalDeviceFeatures>
      phys_device_features_;
  std::map<::VkPhysicalDevice, std::vector<::VkQueueFamilyProperties>>
      phys_device_queue_family_properties_;
  std::map<::VkPhysicalDevice, std::vector<::VkExtensionProperties>>
      supported_device_extension_properties_;

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
      ::VkDebugUtilsMessageTypeFlagsEXT message_type,
      const ::VkDebugUtilsMessengerCallbackDataEXT* data, void*) {
    CHECK_PRECONDITION(
        data->sType ==
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT);
    std::print("[{}] <{}> {}\n", impl::ConvertToString(message_severity),
               data->pMessageIdName, data->pMessage);
    // std::cout << "Queue Labels: \n";
    // for (std::uint32_t i = 0; i < data->queueLabelCount; ++i) {
    //   const ::VkDebugUtilsLabelEXT& _ = data->pQueueLabels[i];
    //   std::cout << " .. " << _.pLabelName << '\n';
    // }
    // std::cout << "Command Buffer Labels: \n";
    // for (std::uint32_t i = 0; i < data->cmdBufLabelCount; ++i) {
    //   const ::VkDebugUtilsLabelEXT& _ = data->pCmdBufLabels[i];
    //   std::cout << " :: " << _.pLabelName << '\n';
    // }
    // std::cout << "Object Names: \n";
    // for (std::uint32_t i = 0; i < data->objectCount; ++i) {
    //   const ::VkDebugUtilsObjectNameInfoEXT& _ = data->pObjects[i];
    //   std::cout << " ** " << _.pObjectName << '\n';
    // }
    return VK_FALSE;
  }
};

//------------------------------------------------------------------------------
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
      std::print("Supported Instance Layer: {}\n", property.layerName);
    }

    for (auto&& property : supported_extensions_) {
      std::print("Supported Instance Extension: {}\n", property.extensionName);
    }

    application_info_.pApplicationName = name_.c_str();
    application_info_.applicationVersion = version;
  }

  Instance CreateInstance(std::span<const char*> requested_layers = {},
                          std::span<const char*> requested_extensions = {},
                          DebugLevel debug_level = DebugLevel::NONE) {
    std::vector<const char*> layers{requested_layers.begin(),
                                    requested_layers.end()};
    std::vector<const char*> extensions{requested_extensions.begin(),
                                        requested_extensions.end()};

    if (impl::HasLayerProperty(supported_layers_,
                               impl::VALIDATION_LAYER_NAME)) {
      layers.push_back(impl::VALIDATION_LAYER_NAME);
    }

    if (debug_level != DebugLevel::NONE) {
      if (impl::HasExtensionProperty(supported_extensions_,
                                     impl::DEBUG_EXTENSION_NAME)) {
        extensions.push_back(impl::DEBUG_EXTENSION_NAME);
      } else {
        std::cerr << "Missing debug extension: " << impl::DEBUG_EXTENSION_NAME;
      }
    }

    return Instance{std::addressof(application_info_),  //
                    std::move(layers),                  //
                    std::move(extensions),              //
                    debug_level};
  }

 private:
  ::VkApplicationInfo application_info_ = impl::MakeApplicationInfo();

  std::vector<::VkLayerProperties> supported_layers_;
  std::vector<::VkExtensionProperties> supported_extensions_;
  std::string name_;
};

}  // namespace volcano
