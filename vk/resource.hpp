#pragma once

#include <algorithm>
#include <sstream>
#include <vector>

#include "lib/base.hpp"

namespace volcano::vk {

inline const ::VkAllocationCallbacks* ALLOCATOR = nullptr;

//------------------------------------------------------------------------------

namespace impl {
template <typename VkType>
class MoveOnlyAdapterBase {
 public:
  DECLARE_COPY_DELETE(MoveOnlyAdapterBase);
  DECLARE_MOVE_DEFAULT(MoveOnlyAdapterBase);

  MoveOnlyAdapterBase() = default;
  ~MoveOnlyAdapterBase() = default;

  MoveOnlyAdapterBase(const VkType& that)
      : vk_object_{std::make_unique<VkType>(that)} {}

  operator VkType&() { return *vk_object_; };
  operator const VkType&() { return *vk_object_; };

  VkType& operator()() noexcept { return *vk_object_; };
  const VkType& operator()() const noexcept { return *vk_object_; };

  VkType* address() noexcept { return vk_object_.get(); }
  const VkType* address() const noexcept { return vk_object_.get(); }

 private:
  std::unique_ptr<VkType> vk_object_ = std::make_unique<VkType>();
};

template <typename VkType, ::VkStructureType TypeValue>
class TypeValueAdapterBase : public MoveOnlyAdapterBase<VkType> {
  using BaseType = MoveOnlyAdapterBase<VkType>;

 public:
  TypeValueAdapterBase() {
    static_cast<VkType&>(*this).sType = TypeValue;  //
  }

  TypeValueAdapterBase(const VkType& that) : BaseType{that} {
    static_cast<VkType&>(*this).sType = TypeValue;  //
  }
};
}  // namespace impl

//------------------------------------------------------------------------------

class MemoryRequirements final
    : public impl::MoveOnlyAdapterBase<::VkMemoryRequirements> {};

//------------------------------------------------------------------------------

class DebugUtilsMessengerCreateInfo final        //
    : public impl::TypeValueAdapterBase<         //
          ::VkDebugUtilsMessengerCreateInfoEXT,  //
          VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT> {};

class ApplicationInfo final               //
    : public impl::TypeValueAdapterBase<  //
          ::VkApplicationInfo,            //
          VK_STRUCTURE_TYPE_APPLICATION_INFO> {};

class InstanceCreateInfo final            //
    : public impl::TypeValueAdapterBase<  //
          ::VkInstanceCreateInfo,         //
          VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO> {};

class DeviceCreateInfo final              //
    : public impl::TypeValueAdapterBase<  //
          ::VkDeviceCreateInfo,           //
          VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO> {};

class DeviceQueueCreateInfo final         //
    : public impl::TypeValueAdapterBase<  //
          ::VkDeviceQueueCreateInfo,      //
          VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO> {};

class BufferCreateInfo final              //
    : public impl::TypeValueAdapterBase<  //
          ::VkBufferCreateInfo,           //
          VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO> {};

class MemoryAllocateInfo final            //
    : public impl::TypeValueAdapterBase<  //
          ::VkMemoryAllocateInfo,         //
          VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO> {};

class CommandPoolCreateInfo final         //
    : public impl::TypeValueAdapterBase<  //
          ::VkCommandPoolCreateInfo,      //
          VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO> {};

class ImageViewCreateInfo final           //
    : public impl::TypeValueAdapterBase<  //
          ::VkImageViewCreateInfo,        //
          VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO> {};

class RenderPassCreateInfo final          //
    : public impl::TypeValueAdapterBase<  //
          ::VkRenderPassCreateInfo,       //
          VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO> {};

class PipelineLayoutCreateInfo final      //
    : public impl::TypeValueAdapterBase<  //
          ::VkPipelineLayoutCreateInfo,   //
          VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO> {};

class ShaderModuleCreateInfo final        //
    : public impl::TypeValueAdapterBase<  //
          ::VkShaderModuleCreateInfo,     //
          VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO> {};

class SwapchainCreateInfo final           //
    : public impl::TypeValueAdapterBase<  //
          ::VkSwapchainCreateInfoKHR,     //
          VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR> {};

//------------------------------------------------------------------------------

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

template <typename EnumeratedType>
class EnumeratedBase {
 public:
  DECLARE_COPY_DELETE(EnumeratedBase);
  DECLARE_MOVE_DEFAULT(EnumeratedBase);

  EnumeratedBase() = default;
  ~EnumeratedBase() = default;

  std::span<EnumeratedType> operator()() { return {enumerated_}; }
  std::span<const EnumeratedType> operator()() const { return {enumerated_}; }

 protected:
  std::vector<EnumeratedType> enumerated_;
};

template <typename EnumeratedType,  //
          auto Enumerate0>
class Enumerator0Base : public EnumeratedBase<EnumeratedType> {
 public:
  Enumerator0Base() {
    MaybeEnumerateProperties(  //
        Enumerate0, InOut(this->enumerated_));
  }
};

template <typename Parameter1Type,  //
          typename EnumeratedType,  //
          auto Enumerate1>
class Enumerator1Base : public EnumeratedBase<EnumeratedType> {
 public:
  explicit Enumerator1Base(Parameter1Type param1) : param1_{param1} {
    MaybeEnumerateProperties(  //
        std::bind_front(Enumerate1, param1_), InOut(this->enumerated_));
  }

 private:
  Parameter1Type param1_;
};

template <typename Parameter1Type,  //
          typename Parameter2Type,  //
          typename EnumeratedType,  //
          auto Enumerate2>
class Enumerator2Base : public EnumeratedBase<EnumeratedType> {
 public:
  explicit Enumerator2Base(Parameter1Type param1, Parameter2Type param2)
      : param1_{param1}, param2_{param2} {
    MaybeEnumerateProperties(  //
        std::bind_front(Enumerate2, param1_, param2_),
        InOut(this->enumerated_));
  }

 private:
  Parameter1Type param1_;
  Parameter2Type param2_;
};

//------------------------------------------------------------------------------

using LayerName = const char*;

using InstanceLayerPropertiesBase =  //
    Enumerator0Base<                 //
        ::VkLayerProperties,         //
        ::vkEnumerateInstanceLayerProperties>;

using InstanceExtensionPropertiesBase =  //
    Enumerator1Base<                     //
        LayerName,                       //
        ::VkExtensionProperties,         //
        ::vkEnumerateInstanceExtensionProperties>;

using PhysicalDevicesBase =  //
    Enumerator1Base<         //
        ::VkInstance,        //
        ::VkPhysicalDevice,  //
        ::vkEnumeratePhysicalDevices>;

using DeviceExtensionPropertiesBase =  //
    Enumerator2Base<                   //
        ::VkPhysicalDevice,            //
        LayerName,                     //
        ::VkExtensionProperties,       //
        ::vkEnumerateDeviceExtensionProperties>;

using PhysicalDeviceQueueFamilyPropertiesBase =  //
    Enumerator1Base<                             //
        ::VkPhysicalDevice,                      //
        ::VkQueueFamilyProperties,               //
        ::vkGetPhysicalDeviceQueueFamilyProperties>;

using PhysicalDeviceSurfaceFormatsBase =  //
    Enumerator2Base<                      //
        ::VkPhysicalDevice,               //
        ::VkSurfaceKHR,                   //
        ::VkSurfaceFormatKHR,             //
        ::vkGetPhysicalDeviceSurfaceFormatsKHR>;

using PhysicalDeviceSurfacePresentModesBase =  //
    Enumerator2Base<                           //
        ::VkPhysicalDevice,                    //
        ::VkSurfaceKHR,                        //
        ::VkPresentModeKHR,                    //
        ::vkGetPhysicalDeviceSurfacePresentModesKHR>;

using SwapchainImagesBase =  //
    Enumerator2Base<         //
        ::VkDevice,          //
        ::VkSwapchainKHR,    //
        ::VkImage,           //
        ::vkGetSwapchainImagesKHR>;

//------------------------------------------------------------------------------

class InstanceLayerProperties final : public InstanceLayerPropertiesBase {
 public:
  using InstanceLayerPropertiesBase::InstanceLayerPropertiesBase;
};

class InstanceExtensionProperties final
    : public InstanceExtensionPropertiesBase {
 public:
  using InstanceExtensionPropertiesBase::InstanceExtensionPropertiesBase;
};

class PhysicalDevices final : public PhysicalDevicesBase {
 public:
  using PhysicalDevicesBase::PhysicalDevicesBase;
};

class DeviceExtensionProperties final : public DeviceExtensionPropertiesBase {
 public:
  using DeviceExtensionPropertiesBase::DeviceExtensionPropertiesBase;
};

class PhysicalDeviceSurfaceFormats final
    : public PhysicalDeviceSurfaceFormatsBase {
 public:
  using PhysicalDeviceSurfaceFormatsBase::PhysicalDeviceSurfaceFormatsBase;
};

class PhysicalDeviceQueueFamilyProperties final
    : public PhysicalDeviceQueueFamilyPropertiesBase {
 public:
  using PhysicalDeviceQueueFamilyPropertiesBase::
      PhysicalDeviceQueueFamilyPropertiesBase;
};

class PhysicalDevicePresentModes final
    : public PhysicalDeviceSurfacePresentModesBase {
 public:
  using PhysicalDeviceSurfacePresentModesBase::
      PhysicalDeviceSurfacePresentModesBase;
};

class SwapchainImages final : public SwapchainImagesBase {
 public:
  using SwapchainImagesBase::SwapchainImagesBase;
};

//------------------------------------------------------------------------------
// NOTE: Extra information (such as the create info, or the source device) is
// retained (eg. on the heap, to avoid copies) for checking and debugging.

//------------------------------------------------------------------------------

template <typename HandleType,                   //
          typename HandleCreateInfoType,         //
          typename HandleCreateInfoAdapterType,  //
          auto CreateHandle,                     //
          auto DestroyHandle>                    //
class HandleBase {
 public:
  DECLARE_COPY_DELETE(HandleBase);

  HandleBase() = delete;
  ~HandleBase() {
    if (handle_ != VK_NULL_HANDLE) {
      DestroyHandle(handle_, ALLOCATOR);
    }
  }

  HandleBase(HandleBase&& that) noexcept
      : handle_{that.handle_}, create_info_{std::move(that.create_info_)} {
    that.handle_ = VK_NULL_HANDLE;
  }

  HandleBase& operator=(HandleBase&& that) noexcept {
    if (this != &that) {
      using std::swap;
      swap(this->handle_, that.handle_);
      swap(this->create_info_, that.create_info_);
    }
    return *this;
  }

  explicit HandleBase(const HandleCreateInfoType& create_info)
      : create_info_{create_info} {
    ::VkResult result = CreateHandle(create_info_.address(), ALLOCATOR,
                                     std::addressof(handle_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

  HandleType handle() { return handle_; }
  const HandleCreateInfoType& create_info() const { return create_info_(); }

 private:
  HandleType handle_ = VK_NULL_HANDLE;
  HandleCreateInfoAdapterType create_info_;
};

//------------------------------------------------------------------------------

template <typename ParentType,                   //
          typename HandleType,                   //
          typename HandleCreateInfoType,         //
          typename HandleCreateInfoAdapterType,  //
          auto CreateHandle,                     //
          auto DestroyHandle>                    //
class ParentedHandleBase {
 public:
  DECLARE_COPY_DELETE(ParentedHandleBase);

  ParentedHandleBase() = delete;
  ~ParentedHandleBase() {
    if (handle_ != VK_NULL_HANDLE) {
      CHECK_INVARIANT(parent_ != VK_NULL_HANDLE);
      DestroyHandle(parent_, handle_, ALLOCATOR);
    }
  }

  ParentedHandleBase(ParentedHandleBase&& that) noexcept
      : parent_{that.parent_},
        handle_{that.handle_},
        create_info_{std::move(that.create_info_)} {
    that.parent_ = VK_NULL_HANDLE;
    that.handle_ = VK_NULL_HANDLE;
  }

  ParentedHandleBase& operator=(ParentedHandleBase&& that) noexcept {
    if (this != &that) {
      using std::swap;
      swap(this->parent_, that.parent_);
      swap(this->handle_, that.handle_);
      swap(this->create_info_, that.create_info_);
    }
    return *this;
  }

  explicit ParentedHandleBase(ParentType parent,
                              const HandleCreateInfoType& create_info)
      : parent_{parent}, create_info_{create_info} {
    ::VkResult result = CreateHandle(parent_, create_info_.address(), ALLOCATOR,
                                     std::addressof(handle_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

  ParentType parent() { return parent_; }
  HandleType handle() { return handle_; }

  const HandleCreateInfoType& create_info() const { return create_info_(); }

 private:
  ParentType parent_ = VK_NULL_HANDLE;
  HandleType handle_ = VK_NULL_HANDLE;
  HandleCreateInfoAdapterType create_info_;
};

//------------------------------------------------------------------------------

using InstanceBase =             //
    HandleBase<                  //
        ::VkInstance,            //
        ::VkInstanceCreateInfo,  //
        InstanceCreateInfo,      //
        ::vkCreateInstance,      //
        ::vkDestroyInstance>;

using BufferBase =             //
    ParentedHandleBase<        //
        ::VkDevice,            //
        ::VkBuffer,            //
        ::VkBufferCreateInfo,  //
        BufferCreateInfo,      //
        ::vkCreateBuffer,      //
        ::vkDestroyBuffer>;

using DeviceMemoryBase =         //
    ParentedHandleBase<          //
        ::VkDevice,              //
        ::VkDeviceMemory,        //
        ::VkMemoryAllocateInfo,  //
        MemoryAllocateInfo,      //
        ::vkAllocateMemory,      //
        ::vkFreeMemory>;

using CommandPoolBase =             //
    ParentedHandleBase<             //
        ::VkDevice,                 //
        ::VkCommandPool,            //
        ::VkCommandPoolCreateInfo,  //
        CommandPoolCreateInfo,      //
        ::vkCreateCommandPool,      //
        ::vkDestroyCommandPool>;

using ImageViewBase =             //
    ParentedHandleBase<           //
        ::VkDevice,               //
        ::VkImageView,            //
        ::VkImageViewCreateInfo,  //
        ImageViewCreateInfo,      //
        ::vkCreateImageView,      //
        ::vkDestroyImageView>;

using RenderPassBase =             //
    ParentedHandleBase<            //
        ::VkDevice,                //
        ::VkRenderPass,            //
        ::VkRenderPassCreateInfo,  //
        RenderPassCreateInfo,      //
        ::vkCreateRenderPass,      //
        ::vkDestroyRenderPass>;

using PipelineLayoutBase =             //
    ParentedHandleBase<                //
        ::VkDevice,                    //
        ::VkPipelineLayout,            //
        ::VkPipelineLayoutCreateInfo,  //
        PipelineLayoutCreateInfo,      //
        ::vkCreatePipelineLayout,      //
        ::vkDestroyPipelineLayout>;

using ShaderModuleBase =             //
    ParentedHandleBase<              //
        ::VkDevice,                  //
        ::VkShaderModule,            //
        ::VkShaderModuleCreateInfo,  //
        ShaderModuleCreateInfo,      //
        ::vkCreateShaderModule,      //
        ::vkDestroyShaderModule>;

using SwapchainBase =                //
    ParentedHandleBase<              //
        ::VkDevice,                  //
        ::VkSwapchainKHR,            //
        ::VkSwapchainCreateInfoKHR,  //
        SwapchainCreateInfo,         //
        ::vkCreateSwapchainKHR,      //
        ::vkDestroySwapchainKHR>;

//------------------------------------------------------------------------------

class Instance final : public InstanceBase {
 public:
  using InstanceBase::InstanceBase;
};

class Buffer final : public BufferBase {
 public:
  using BufferBase::BufferBase;
};

class DeviceMemory final : public DeviceMemoryBase {
 public:
  using DeviceMemoryBase::DeviceMemoryBase;
};

class CommandPool final : public CommandPoolBase {
 public:
  using CommandPoolBase::CommandPoolBase;
};

class ImageView final : public ImageViewBase {
 public:
  using ImageViewBase::ImageViewBase;
};

class RenderPass final : public RenderPassBase {
 public:
  using RenderPassBase::RenderPassBase;
};

class PipelineLayout final : public PipelineLayoutBase {
 public:
  using PipelineLayoutBase::PipelineLayoutBase;
};

class ShaderModule final : public ShaderModuleBase {
 public:
  using ShaderModuleBase::ShaderModuleBase;
};

class Swapchain final : public SwapchainBase {
 public:
  using SwapchainBase::SwapchainBase;
};

//------------------------------------------------------------------------------

class Device final {
 public:
  DECLARE_COPY_DELETE(Device);

  Device() = delete;
  ~Device() {
    if (device_ != VK_NULL_HANDLE) {
      ::vkDestroyDevice(device_, ALLOCATOR);
    }
  }

  Device(Device&& that) noexcept
      : device_{that.device_},
        phys_device_{that.phys_device_},
        create_info_{std::move(that.create_info_)} {
    that.device_ = VK_NULL_HANDLE;
    that.phys_device_ = VK_NULL_HANDLE;
  }

  Device& operator=(Device&& that) noexcept {
    if (this != &that) {
      using std::swap;
      swap(this->device_, that.device_);
      swap(this->phys_device_, that.phys_device_);
      swap(this->create_info_, that.create_info_);
    }
    return *this;
  }

  explicit Device(::VkPhysicalDevice phys_device,
                  const ::VkDeviceCreateInfo& create_info)
      : phys_device_{phys_device}, create_info_{create_info} {
    ::VkResult result =
        ::vkCreateDevice(phys_device_, create_info_.address(), vk::ALLOCATOR,
                         std::addressof(device_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

  ::VkDevice handle() { return device_; }
  ::VkPhysicalDevice phys_device() { return phys_device_; }
  const ::VkDeviceCreateInfo& create_info() const { return create_info_(); }

 private:
  ::VkDevice device_ = VK_NULL_HANDLE;
  ::VkPhysicalDevice phys_device_ = VK_NULL_HANDLE;
  DeviceCreateInfo create_info_;
};

//------------------------------------------------------------------------------

class Queue final {
 public:
  DECLARE_COPY_DELETE(Queue);
  DECLARE_MOVE_DEFAULT(Queue);

  Queue() = delete;
  ~Queue() = default;

  explicit Queue(::VkDevice device,                 //
                 std::uint32_t queue_family_index,  //
                 std::uint32_t queue_index)
      : device_{device},
        queue_family_index_{queue_family_index},
        queue_index_{queue_index} {
    CHECK_PRECONDITION(device_ != VK_NULL_HANDLE);
    ::vkGetDeviceQueue(device_, queue_family_index_, queue_index_,
                       std::addressof(queue_));
  }

  ::VkDevice device() { return device_; }
  ::VkQueue handle() { return queue_; }

  std::uint32_t family_index() const { return queue_family_index_; }
  std::uint32_t index() const { return queue_index_; }

 private:
  ::VkDevice device_ = VK_NULL_HANDLE;
  ::VkQueue queue_ = VK_NULL_HANDLE;

  std::uint32_t queue_family_index_ = std::numeric_limits<std::uint32_t>::max();
  std::uint32_t queue_index_ = std::numeric_limits<std::uint32_t>::max();
};

//------------------------------------------------------------------------------

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

inline bool HasStringName(const std::vector<const char*>& names,
                          std::string_view target) {
  return std::any_of(names.begin(), names.end(), [target](const char* name) {
    return static_cast<std::string_view>(name) == target;
  });
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

template <typename TargetFunctionPointer>
inline void LoadInstanceFunction(const char* name, ::VkInstance instance,
                                 Out<TargetFunctionPointer> target) {
  CHECK_PRECONDITION(instance != VK_NULL_HANDLE);
  *target = reinterpret_cast<TargetFunctionPointer>(
      ::vkGetInstanceProcAddr(instance, name));
}

//------------------------------------------------------------------------------

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

}  // namespace volcano::vk
