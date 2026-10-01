#pragma once

#include <algorithm>
#include <sstream>
#include <vector>

#include "engine/base.hpp"

#include <vulkan/vulkan.h>

static_assert(VK_HEADER_VERSION >= 290, "Update vulkan header version.");

namespace volcano::vk {

//------------------------------------------------------------------------------

inline const ::VkAllocationCallbacks* ALLOCATOR = nullptr;

//------------------------------------------------------------------------------
// NOTE: Copies of data used to create objects (*`CreateInfo`s) are retained for
// checking and debugging purposes. This metadata is kept on the heap in
// move-only form to avoid copies and maintain stable addresses.

namespace internal {
template <typename VkType>
class BoxAdapterBase {
 public:
  DECLARE_COPY_DELETE(BoxAdapterBase);
  DECLARE_MOVE_DEFAULT(BoxAdapterBase);

  BoxAdapterBase() = default;
  ~BoxAdapterBase() = default;

  BoxAdapterBase(const VkType& that)
      : vk_object_{std::make_unique<VkType>(that)} {}

  operator VkType&() { return *vk_object_; };
  operator const VkType&() { return *vk_object_; };

  auto operator()() noexcept -> VkType& { return *vk_object_; };
  auto operator()() const noexcept -> const VkType& { return *vk_object_; };

  auto address() noexcept -> VkType* { return vk_object_.get(); }
  auto address() const noexcept -> const VkType* { return vk_object_.get(); }

 private:
  std::unique_ptr<VkType> vk_object_ = std::make_unique<VkType>();
};

template <typename VkType, ::VkStructureType TypeValue>
class TypeValueAdapterBase : public BoxAdapterBase<VkType> {
  using BaseType = BoxAdapterBase<VkType>;

 public:
  TypeValueAdapterBase() {
    this->address()->sType = TypeValue;  //
  }

  TypeValueAdapterBase(const VkType& that) : BaseType{that} {
    this->address()->sType = TypeValue;  //
  }
};
}  // namespace internal

//------------------------------------------------------------------------------

class DebugUtilsMessengerCreateInfo final        //
    : public internal::TypeValueAdapterBase<     //
          ::VkDebugUtilsMessengerCreateInfoEXT,  //
          VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT> {};

class ApplicationInfo final                   //
    : public internal::TypeValueAdapterBase<  //
          ::VkApplicationInfo,                //
          VK_STRUCTURE_TYPE_APPLICATION_INFO> {};

class InstanceCreateInfo final                //
    : public internal::TypeValueAdapterBase<  //
          ::VkInstanceCreateInfo,             //
          VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO> {};

class DeviceCreateInfo final                  //
    : public internal::TypeValueAdapterBase<  //
          ::VkDeviceCreateInfo,               //
          VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO> {};

class DeviceQueueCreateInfo final             //
    : public internal::TypeValueAdapterBase<  //
          ::VkDeviceQueueCreateInfo,          //
          VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO> {};

class SubmitInfo final                        //
    : public internal::TypeValueAdapterBase<  //
          ::VkSubmitInfo,                     //
          VK_STRUCTURE_TYPE_SUBMIT_INFO> {};

class SemaphoreCreateInfo final               //
    : public internal::TypeValueAdapterBase<  //
          ::VkSemaphoreCreateInfo,            //
          VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO> {};

class FenceCreateInfo final                   //
    : public internal::TypeValueAdapterBase<  //
          ::VkFenceCreateInfo,                //
          VK_STRUCTURE_TYPE_FENCE_CREATE_INFO> {};

class BufferCreateInfo final                  //
    : public internal::TypeValueAdapterBase<  //
          ::VkBufferCreateInfo,               //
          VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO> {};

class MemoryAllocateInfo final                //
    : public internal::TypeValueAdapterBase<  //
          ::VkMemoryAllocateInfo,             //
          VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO> {};

class CommandBufferAllocateInfo final         //
    : public internal::TypeValueAdapterBase<  //
          ::VkCommandBufferAllocateInfo,      //
          VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO> {};

class CommandBufferBeginInfo final            //
    : public internal::TypeValueAdapterBase<  //
          ::VkCommandBufferBeginInfo,         //
          VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO> {};

class CommandPoolCreateInfo final             //
    : public internal::TypeValueAdapterBase<  //
          ::VkCommandPoolCreateInfo,          //
          VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO> {};

class ImageViewCreateInfo final               //
    : public internal::TypeValueAdapterBase<  //
          ::VkImageViewCreateInfo,            //
          VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO> {};

class PipelineLayoutCreateInfo final          //
    : public internal::TypeValueAdapterBase<  //
          ::VkPipelineLayoutCreateInfo,       //
          VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO> {};

class ShaderModuleCreateInfo final            //
    : public internal::TypeValueAdapterBase<  //
          ::VkShaderModuleCreateInfo,         //
          VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO> {};

class SwapchainCreateInfo final               //
    : public internal::TypeValueAdapterBase<  //
          ::VkSwapchainCreateInfoKHR,         //
          VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR> {};

class PresentInfo final                       //
    : public internal::TypeValueAdapterBase<  //
          ::VkPresentInfoKHR,                 //
          VK_STRUCTURE_TYPE_PRESENT_INFO_KHR> {};

class FramebufferCreateInfo final             //
    : public internal::TypeValueAdapterBase<  //
          ::VkFramebufferCreateInfo,          //
          VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO> {};

class GraphicsPipelineCreateInfo final        //
    : public internal::TypeValueAdapterBase<  //
          ::VkGraphicsPipelineCreateInfo,     //
          VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO> {};

class PipelineShaderStageCreateInfo final     //
    : public internal::TypeValueAdapterBase<  //
          ::VkPipelineShaderStageCreateInfo,  //
          VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO> {};

class RenderPassCreateInfo final              //
    : public internal::TypeValueAdapterBase<  //
          ::VkRenderPassCreateInfo,           //
          VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO> {};

class RenderPassBeginInfo final               //
    : public internal::TypeValueAdapterBase<  //
          ::VkRenderPassBeginInfo,            //
          VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO> {};

//------------------------------------------------------------------------------

namespace internal {
template <typename PropertyType>
class QueriedPropertyBase {
 public:
  DECLARE_COPY_DELETE(QueriedPropertyBase);
  DECLARE_MOVE_DEFAULT(QueriedPropertyBase);

  QueriedPropertyBase() = default;
  ~QueriedPropertyBase() = default;

  explicit QueriedPropertyBase(const PropertyType& property)
      : property_{property} {}

  operator PropertyType&() { return property_; }
  operator const PropertyType&() const { return property_; }

  auto operator()() -> PropertyType& { return property_; }
  auto operator()() const -> const PropertyType& { return property_; }

  auto address() -> PropertyType* { return std::addressof(property_); }
  auto address() const -> const PropertyType* {
    return std::addressof(property_);
  }

 protected:
  PropertyType property_;
};

template <typename PropertyType,  //
          auto Query0>
class PropertyQuerier0Base : public QueriedPropertyBase<PropertyType> {
  using BaseType = QueriedPropertyBase<PropertyType>;

 public:
  explicit PropertyQuerier0Base(const PropertyType& property)
      : BaseType{property} {}

  PropertyQuerier0Base() { Query0(std::addressof(this->property_)); }
};

template <typename Parameter1Type,  //
          typename PropertyType,    //
          auto Query1>
class PropertyQuerier1Base : public QueriedPropertyBase<PropertyType> {
  using BaseType = QueriedPropertyBase<PropertyType>;

 public:
  PropertyQuerier1Base() = default;

  explicit PropertyQuerier1Base(const PropertyType& property)
      : BaseType{property} {}

  explicit PropertyQuerier1Base(Parameter1Type param1)
      : param1_{std::move(param1)} {
    Query1(param1_, std::addressof(this->property_));
  }

 private:
  Parameter1Type param1_;
};

template <typename Parameter1Type,  //
          typename Parameter2Type,  //
          typename PropertyType,    //
          auto Query2>
class PropertyQuerier2Base : public QueriedPropertyBase<PropertyType> {
  using BaseType = QueriedPropertyBase<PropertyType>;

 public:
  PropertyQuerier2Base() = default;

  explicit PropertyQuerier2Base(const PropertyType& property)
      : BaseType{property} {}

  explicit PropertyQuerier2Base(Parameter1Type param1, Parameter2Type param2)
      : param1_{std::move(param1)},  //
        param2_{std::move(param2)} {
    Query2(param1_, param2_, std::addressof(this->property_));
  }

 private:
  Parameter1Type param1_;
  Parameter2Type param2_;
};
}  // namespace internal

//------------------------------------------------------------------------------

using MemoryRequirementsBase =       //
    internal::PropertyQuerier2Base<  //
        ::VkDevice,                  //
        ::VkBuffer,                  //
        ::VkMemoryRequirements,      //
        ::vkGetBufferMemoryRequirements>;

using PhysicalDevicePropertiesBase =   //
    internal::PropertyQuerier1Base<    //
        ::VkPhysicalDevice,            //
        ::VkPhysicalDeviceProperties,  //
        ::vkGetPhysicalDeviceProperties>;

using PhysicalDeviceMemoryPropertiesBase =   //
    internal::PropertyQuerier1Base<          //
        ::VkPhysicalDevice,                  //
        ::VkPhysicalDeviceMemoryProperties,  //
        ::vkGetPhysicalDeviceMemoryProperties>;

using PhysicalDeviceFeaturesBase =   //
    internal::PropertyQuerier1Base<  //
        ::VkPhysicalDevice,          //
        ::VkPhysicalDeviceFeatures,  //
        ::vkGetPhysicalDeviceFeatures>;

using PhysicalDeviceSurfaceCapabilitiesBase =  //
    internal::PropertyQuerier2Base<            //
        ::VkPhysicalDevice,                    //
        ::VkSurfaceKHR,                        //
        ::VkSurfaceCapabilitiesKHR,            //
        ::vkGetPhysicalDeviceSurfaceCapabilitiesKHR>;

//------------------------------------------------------------------------------

DERIVE_FINAL_WITH_CONSTRUCTORS(MemoryRequirements,  //
                               MemoryRequirementsBase);

DERIVE_FINAL_WITH_CONSTRUCTORS(PhysicalDeviceProperties,  //
                               PhysicalDevicePropertiesBase);

DERIVE_FINAL_WITH_CONSTRUCTORS(PhysicalDeviceMemoryProperties,  //
                               PhysicalDeviceMemoryPropertiesBase);

DERIVE_FINAL_WITH_CONSTRUCTORS(PhysicalDeviceFeatures,  //
                               PhysicalDeviceFeaturesBase);

DERIVE_FINAL_WITH_CONSTRUCTORS(PhysicalDeviceSurfaceCapabilities,  //
                               PhysicalDeviceSurfaceCapabilitiesBase);

//------------------------------------------------------------------------------

namespace internal {
template <typename EnumeratorType, typename PropertyType>
inline auto maybe_enumerate_properties(
    EnumeratorType&& enumerate, InOut<std::vector<PropertyType>> properties)
    -> void {
  std::vector<PropertyType> current;

  // The count can change between the two calls. Retry on VK_INCOMPLETE, and
  // keep only as many entries as the second call wrote.
  for (bool incomplete = true; incomplete;) {
    incomplete = false;

    // Gather the required array size.
    std::uint32_t count = 0;
    invoke_with_continuation(
        Overloaded(
            [](::VkResult result) { CHECK_INVARIANT(result == VK_SUCCESS); },
            []() {}),
        enumerate, std::addressof(count), nullptr);
    current.resize(count);
    if (!count) {
      break;
    }

    // Gather the enumerated properties.
    invoke_with_continuation(
        Overloaded(
            [&incomplete](::VkResult result) {
              CHECK_INVARIANT(result == VK_SUCCESS || result == VK_INCOMPLETE);
              incomplete = (result == VK_INCOMPLETE);
            },
            []() {}),
        enumerate, std::addressof(count), current.data());
    current.resize(count);
  }

  properties->insert(properties->end(),  //
                     current.begin(),    //
                     current.end());
}

template <typename PropertyType>
class EnumeratedPropertyBase {
 public:
  DECLARE_COPY_DELETE(EnumeratedPropertyBase);
  DECLARE_MOVE_DEFAULT(EnumeratedPropertyBase);

  EnumeratedPropertyBase() = default;
  ~EnumeratedPropertyBase() = default;

  explicit EnumeratedPropertyBase(std::span<PropertyType> properties)
      : properties_(properties.begin(), properties.end()) {}

  operator std::span<PropertyType>() { return {properties_}; }
  operator std::span<const PropertyType>() const { return {properties_}; }

  auto operator()() -> std::span<PropertyType> { return {properties_}; }
  auto operator()() const -> std::span<const PropertyType> {
    return {properties_};
  }

 protected:
  std::vector<PropertyType> properties_;
};

template <typename PropertyType,  //
          auto Enumerate0>
class PropertyEnumerator0Base : public EnumeratedPropertyBase<PropertyType> {
  using BaseType = EnumeratedPropertyBase<PropertyType>;

 public:
  explicit PropertyEnumerator0Base(std::span<PropertyType> properties)
      : BaseType{std::move(properties)} {}

  PropertyEnumerator0Base() {
    maybe_enumerate_properties(  //
        Enumerate0, InOut(this->properties_));
  }
};

template <typename Parameter1Type,  //
          typename PropertyType,    //
          auto Enumerate1>
class PropertyEnumerator1Base : public EnumeratedPropertyBase<PropertyType> {
  using BaseType = EnumeratedPropertyBase<PropertyType>;

 public:
  PropertyEnumerator1Base() = default;

  explicit PropertyEnumerator1Base(std::span<PropertyType> properties)
      : BaseType{std::move(properties)} {}

  explicit PropertyEnumerator1Base(Parameter1Type param1)
      : param1_{std::move(param1)} {
    maybe_enumerate_properties(  //
        std::bind_front(Enumerate1, param1_), InOut(this->properties_));
  }

 private:
  Parameter1Type param1_;
};

template <typename Parameter1Type,  //
          typename Parameter2Type,  //
          typename PropertyType,    //
          auto Enumerate2>
class PropertyEnumerator2Base : public EnumeratedPropertyBase<PropertyType> {
  using BaseType = EnumeratedPropertyBase<PropertyType>;

 public:
  PropertyEnumerator2Base() = default;

  explicit PropertyEnumerator2Base(std::span<PropertyType> properties)
      : BaseType{std::move(properties)} {}

  explicit PropertyEnumerator2Base(Parameter1Type param1, Parameter2Type param2)
      : param1_{std::move(param1)},  //
        param2_{std::move(param2)} {
    maybe_enumerate_properties(  //
        std::bind_front(Enumerate2, param1_, param2_),
        InOut(this->properties_));
  }

 private:
  Parameter1Type param1_;
  Parameter2Type param2_;
};
}  // namespace internal

//------------------------------------------------------------------------------

using LayerName = const char*;

using InstanceLayerPropertiesBase =     //
    internal::PropertyEnumerator0Base<  //
        ::VkLayerProperties,            //
        ::vkEnumerateInstanceLayerProperties>;

using InstanceExtensionPropertiesBase =  //
    internal::PropertyEnumerator1Base<   //
        LayerName,                       //
        ::VkExtensionProperties,         //
        ::vkEnumerateInstanceExtensionProperties>;

using PhysicalDevicesBase =             //
    internal::PropertyEnumerator1Base<  //
        ::VkInstance,                   //
        ::VkPhysicalDevice,             //
        ::vkEnumeratePhysicalDevices>;

using DeviceExtensionPropertiesBase =   //
    internal::PropertyEnumerator2Base<  //
        ::VkPhysicalDevice,             //
        LayerName,                      //
        ::VkExtensionProperties,        //
        ::vkEnumerateDeviceExtensionProperties>;

using PhysicalDeviceQueueFamilyPropertiesBase =  //
    internal::PropertyEnumerator1Base<           //
        ::VkPhysicalDevice,                      //
        ::VkQueueFamilyProperties,               //
        ::vkGetPhysicalDeviceQueueFamilyProperties>;

using PhysicalDeviceSurfaceFormatsBase =  //
    internal::PropertyEnumerator2Base<    //
        ::VkPhysicalDevice,               //
        ::VkSurfaceKHR,                   //
        ::VkSurfaceFormatKHR,             //
        ::vkGetPhysicalDeviceSurfaceFormatsKHR>;

using PhysicalDeviceSurfacePresentModesBase =  //
    internal::PropertyEnumerator2Base<         //
        ::VkPhysicalDevice,                    //
        ::VkSurfaceKHR,                        //
        ::VkPresentModeKHR,                    //
        ::vkGetPhysicalDeviceSurfacePresentModesKHR>;

using SwapchainImagesBase =             //
    internal::PropertyEnumerator2Base<  //
        ::VkDevice,                     //
        ::VkSwapchainKHR,               //
        ::VkImage,                      //
        ::vkGetSwapchainImagesKHR>;

//------------------------------------------------------------------------------
DERIVE_FINAL_WITH_CONSTRUCTORS(InstanceLayerProperties,  //
                               InstanceLayerPropertiesBase);

DERIVE_FINAL_WITH_CONSTRUCTORS(InstanceExtensionProperties,  //
                               InstanceExtensionPropertiesBase);

DERIVE_FINAL_WITH_CONSTRUCTORS(PhysicalDevices,  //
                               PhysicalDevicesBase);

DERIVE_FINAL_WITH_CONSTRUCTORS(DeviceExtensionProperties,  //
                               DeviceExtensionPropertiesBase);

DERIVE_FINAL_WITH_CONSTRUCTORS(PhysicalDeviceQueueFamilyProperties,  //
                               PhysicalDeviceQueueFamilyPropertiesBase);

DERIVE_FINAL_WITH_CONSTRUCTORS(PhysicalDeviceSurfaceFormats,  //
                               PhysicalDeviceSurfaceFormatsBase);

DERIVE_FINAL_WITH_CONSTRUCTORS(PhysicalDeviceSurfacePresentModes,  //
                               PhysicalDeviceSurfacePresentModesBase);

DERIVE_FINAL_WITH_CONSTRUCTORS(SwapchainImages,  //
                               SwapchainImagesBase);

//------------------------------------------------------------------------------
namespace internal {
template <typename HandleType, typename HandleOpenInfoType, auto OpenHandle>
inline auto open_handle_default_adapter(const HandleOpenInfoType& info,
                                        HandleType& handle) -> ::VkResult {
  return OpenHandle(std::addressof(info), ALLOCATOR, std::addressof(handle));
}
template <typename HandleType, auto CloseHandle>
inline auto close_handle_default_adapter(const HandleType& handle) -> void {
  CloseHandle(handle, ALLOCATOR);
}

template <typename HandleType,                //
          typename HandleOpenInfoType,        //
          typename HandleOpenInfoHolderType,  //
          auto OpenHandle,                    //
          auto CloseHandle>                   //
class HandleBase {
 public:
  DECLARE_COPY_DELETE(HandleBase);

  HandleBase() = default;
  ~HandleBase() {
    if (handle_) {
      CloseHandle(handle_);
    }
  }

  HandleBase(HandleBase&& that) noexcept
      : handle_{std::exchange(that.handle_, nullptr)},
        info_{std::move(that.info_)} {}

  auto operator=(HandleBase&& that) noexcept -> HandleBase& {
    if (this != &that) {
      if (handle_) {
        CloseHandle(handle_);
      }
      handle_ = std::exchange(that.handle_, nullptr);
      info_ = std::move(that.info_);
    }
    return *this;
  }

  // Adopt Constructor.
  explicit HandleBase(HandleType handle) : handle_{std::move(handle)} {}

  // Adapt Constructor.
  explicit HandleBase(HandleType handle, const HandleOpenInfoType& info)
      : handle_{std::move(handle)}, info_{info} {
    CHECK_PRECONDITION(handle_);
    ::VkResult result = OpenHandle(info_(), handle_);
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

  // Init Constructor.
  explicit HandleBase(const HandleOpenInfoType& info) : info_{info} {
    ::VkResult result = OpenHandle(info_(), handle_);
    CHECK_POSTCONDITION(result == VK_SUCCESS);
    CHECK_POSTCONDITION(handle_);
  }

  // Empty Init Constructor.
  HandleBase(std::nullptr_t) {}

  explicit operator bool() const { return handle_; }
  operator HandleType() const { return handle_; }

  auto handle() const -> const HandleType& { return handle_; }
  auto info() const -> const HandleOpenInfoType& { return info_(); }

 private:
  HandleType handle_{};
  HandleOpenInfoHolderType info_;
};

template <typename HandleType,                //
          typename HandleOpenInfoType,        //
          typename HandleOpenInfoHolderType,  //
          auto OpenHandle,                    //
          auto CloseHandle>                   //
using DefaultHandleResourceBase =             //
    HandleBase<                               //
        HandleType,                           //
        HandleOpenInfoType,                   //
        HandleOpenInfoHolderType,             //
        open_handle_default_adapter<          //
            HandleType,                       //
            HandleOpenInfoType,               //
            OpenHandle>,                      //
        close_handle_default_adapter<         //
            HandleType,                       //
            CloseHandle>>;

//------------------------------------------------------------------------------

template <typename ParentType,          //
          typename HandleType,          //
          typename HandleOpenInfoType,  //
          auto OpenHandle>
inline auto open_parented_handle_default_adapter(const ParentType& parent,
                                                 const HandleOpenInfoType& info,
                                                 HandleType& handle)
    -> ::VkResult {
  return OpenHandle(parent, std::addressof(info), ALLOCATOR,
                    std::addressof(handle));
}
template <typename ParentType,  //
          typename HandleType,  //
          auto CloseHandle>
inline auto close_parented_handle_default_adapter(const ParentType& parent,
                                                  const HandleType& handle)
    -> void {
  CloseHandle(parent, handle, ALLOCATOR);
}

template <typename ParentType,                //
          typename HandleType,                //
          typename HandleOpenInfoType,        //
          typename HandleOpenInfoHolderType,  //
          auto OpenHandle,                    //
          auto CloseHandle>                   //
class ParentedHandleBase {
 public:
  DECLARE_COPY_DELETE(ParentedHandleBase);

  ParentedHandleBase() = default;
  ~ParentedHandleBase() {
    if (handle_) {
      CloseHandle(parent_, handle_);
    }
  }

  ParentedHandleBase(ParentedHandleBase&& that) noexcept
      : parent_{std::exchange(that.parent_, nullptr)},
        handle_{std::exchange(that.handle_, nullptr)},
        info_{std::move(that.info_)} {}

  auto operator=(ParentedHandleBase&& that) noexcept -> ParentedHandleBase& {
    if (this != &that) {
      if (handle_) {
        CloseHandle(parent_, handle_);
      }
      parent_ = std::exchange(that.parent_, nullptr);
      handle_ = std::exchange(that.handle_, nullptr);
      info_ = std::move(that.info_);
    }
    return *this;
  }

  // Adopt Constructor.
  explicit ParentedHandleBase(ParentType parent, HandleType handle)
      : parent_{std::move(parent)}, handle_{std::move(handle)} {}

  // Adapt Constructor.
  explicit ParentedHandleBase(ParentType parent, HandleType handle,
                              const HandleOpenInfoType& info)
      : parent_{std::move(parent)}, handle_{std::move(handle)}, info_{info} {
    CHECK_PRECONDITION(parent_);
    CHECK_PRECONDITION(handle_);
    ::VkResult result = OpenHandle(parent_, info_(), handle_);
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

  // Init Constructor.
  explicit ParentedHandleBase(ParentType parent, const HandleOpenInfoType& info)
      : parent_{std::move(parent)}, info_{info} {
    CHECK_PRECONDITION(parent_);
    ::VkResult result = OpenHandle(parent_, info_(), handle_);
    CHECK_POSTCONDITION(result == VK_SUCCESS);
    CHECK_POSTCONDITION(handle_);
  }

  // Empty Init Constructor.
  ParentedHandleBase(std::nullptr_t _) {}

  explicit operator bool() const { return parent_ && handle_; }
  operator HandleType() const { return handle_; }

  auto parent() const -> const ParentType& { return parent_; }
  auto handle() const -> const HandleType& { return handle_; }

  auto info() const -> const HandleOpenInfoType& { return info_(); }

 private:
  ParentType parent_ = VK_NULL_HANDLE;
  HandleType handle_ = VK_NULL_HANDLE;
  HandleOpenInfoHolderType info_;
};

template <typename ParentType,                  //
          typename HandleType,                  //
          typename HandleOpenInfoType,          //
          typename HandleOpenInfoHolderType,    //
          auto OpenHandle,                      //
          auto CloseHandle>                     //
using DefaultParentedHandleResourceBase =       //
    ParentedHandleBase<                         //
        ParentType,                             //
        HandleType,                             //
        HandleOpenInfoType,                     //
        HandleOpenInfoHolderType,               //
        open_parented_handle_default_adapter<   //
            ParentType,                         //
            HandleType,                         //
            HandleOpenInfoType,                 //
            OpenHandle>,                        //
        close_parented_handle_default_adapter<  //
            ParentType,                         //
            HandleType,                         //
            CloseHandle>>;

}  // namespace internal

//------------------------------------------------------------------------------

using InstanceBase =                      //
    internal::DefaultHandleResourceBase<  //
        ::VkInstance,                     //
        ::VkInstanceCreateInfo,           //
        InstanceCreateInfo,               //
        ::vkCreateInstance,               //
        ::vkDestroyInstance>;

namespace internal {
inline auto end_device_adapter(::VkPhysicalDevice /*physical*/,
                               ::VkDevice device) -> void {
  // This runs from a destructor, so it must not throw. A lost device fails the
  // wait but must still be destroyed.
  [[maybe_unused]] ::VkResult result = ::vkDeviceWaitIdle(device);
  ::vkDestroyDevice(device, ALLOCATOR);
}
}  // namespace internal

using DeviceBase =                                       //
    internal::ParentedHandleBase<                        //
        ::VkPhysicalDevice,                              //
        ::VkDevice,                                      //
        ::VkDeviceCreateInfo,                            //
        DeviceCreateInfo,                                //
        internal::open_parented_handle_default_adapter<  //
            ::VkPhysicalDevice,                          //
            ::VkDevice,                                  //
            ::VkDeviceCreateInfo,                        //
            ::vkCreateDevice>,                           //
        internal::end_device_adapter>;

using SemaphoreBase =                             //
    internal::DefaultParentedHandleResourceBase<  //
        ::VkDevice,                               //
        ::VkSemaphore,                            //
        ::VkSemaphoreCreateInfo,                  //
        SemaphoreCreateInfo,                      //
        ::vkCreateSemaphore,                      //
        ::vkDestroySemaphore>;

using FenceBase =                                 //
    internal::DefaultParentedHandleResourceBase<  //
        ::VkDevice,                               //
        ::VkFence,                                //
        ::VkFenceCreateInfo,                      //
        FenceCreateInfo,                          //
        ::vkCreateFence,                          //
        ::vkDestroyFence>;

using BufferBase =                                //
    internal::DefaultParentedHandleResourceBase<  //
        ::VkDevice,                               //
        ::VkBuffer,                               //
        ::VkBufferCreateInfo,                     //
        BufferCreateInfo,                         //
        ::vkCreateBuffer,                         //
        ::vkDestroyBuffer>;

using DeviceMemoryBase =                          //
    internal::DefaultParentedHandleResourceBase<  //
        ::VkDevice,                               //
        ::VkDeviceMemory,                         //
        ::VkMemoryAllocateInfo,                   //
        MemoryAllocateInfo,                       //
        ::vkAllocateMemory,                       //
        ::vkFreeMemory>;

struct QueueIndex final {
  std::uint32_t family_index = std::numeric_limits<std::uint32_t>::max();
  std::uint32_t index = std::numeric_limits<std::uint32_t>::max();

  auto operator()() noexcept -> QueueIndex& { return *this; };
  auto operator()() const noexcept -> const QueueIndex& { return *this; };

  auto address() noexcept -> QueueIndex* { return this; }
  auto address() const noexcept -> const QueueIndex* { return this; }
};

namespace internal {
inline auto begin_device_queue_adapter(  //
    ::VkDevice device,                   //
    QueueIndex queue,                    //
    ::VkQueue& handle) -> ::VkResult {
  ::vkGetDeviceQueue(device, queue.family_index, queue.index,
                     std::addressof(handle));
  return VK_SUCCESS;
}
inline auto end_device_queue_adapter(::VkDevice, ::VkQueue) -> void {}
}  // namespace internal

using QueueBase =                              //
    internal::ParentedHandleBase<              //
        ::VkDevice,                            //
        ::VkQueue,                             //
        QueueIndex,                            //
        QueueIndex,                            //
        internal::begin_device_queue_adapter,  //
        internal::end_device_queue_adapter>;

namespace internal {
inline auto begin_command_buffer_adapter(    //
    const ::VkCommandBufferBeginInfo& info,  //
    ::VkCommandBuffer handle) -> ::VkResult {
  return ::vkBeginCommandBuffer(handle, std::addressof(info));
}
inline auto end_command_buffer_adapter(::VkCommandBuffer handle) -> void {
  ::vkEndCommandBuffer(handle);
}
}  // namespace internal

using CommandPoolBase =                           //
    internal::DefaultParentedHandleResourceBase<  //
        ::VkDevice,                               //
        ::VkCommandPool,                          //
        ::VkCommandPoolCreateInfo,                //
        CommandPoolCreateInfo,                    //
        ::vkCreateCommandPool,                    //
        ::vkDestroyCommandPool>;

using CommandBufferBuilderBase =                 //
    internal::HandleBase<                        //
        ::VkCommandBuffer,                       //
        ::VkCommandBufferBeginInfo,              //
        CommandBufferBeginInfo,                  //
        internal::begin_command_buffer_adapter,  //
        internal::end_command_buffer_adapter>;

using ImageViewBase =                             //
    internal::DefaultParentedHandleResourceBase<  //
        ::VkDevice,                               //
        ::VkImageView,                            //
        ::VkImageViewCreateInfo,                  //
        ImageViewCreateInfo,                      //
        ::vkCreateImageView,                      //
        ::vkDestroyImageView>;

using RenderPassBase =                            //
    internal::DefaultParentedHandleResourceBase<  //
        ::VkDevice,                               //
        ::VkRenderPass,                           //
        ::VkRenderPassCreateInfo,                 //
        RenderPassCreateInfo,                     //
        ::vkCreateRenderPass,                     //
        ::vkDestroyRenderPass>;

using PipelineLayoutBase =                        //
    internal::DefaultParentedHandleResourceBase<  //
        ::VkDevice,                               //
        ::VkPipelineLayout,                       //
        ::VkPipelineLayoutCreateInfo,             //
        PipelineLayoutCreateInfo,                 //
        ::vkCreatePipelineLayout,                 //
        ::vkDestroyPipelineLayout>;

using ShaderModuleBase =                          //
    internal::DefaultParentedHandleResourceBase<  //
        ::VkDevice,                               //
        ::VkShaderModule,                         //
        ::VkShaderModuleCreateInfo,               //
        ShaderModuleCreateInfo,                   //
        ::vkCreateShaderModule,                   //
        ::vkDestroyShaderModule>;

using SwapchainBase =                             //
    internal::DefaultParentedHandleResourceBase<  //
        ::VkDevice,                               //
        ::VkSwapchainKHR,                         //
        ::VkSwapchainCreateInfoKHR,               //
        SwapchainCreateInfo,                      //
        ::vkCreateSwapchainKHR,                   //
        ::vkDestroySwapchainKHR>;

namespace internal {
inline auto begin_surface_adapter(::VkInstance, Empty, ::VkSurfaceKHR)
    -> ::VkResult {
  CHECK_UNREACHABLE();
  return VK_SUCCESS;
}
}  // namespace internal

using SurfaceBase =                                       //
    internal::ParentedHandleBase<                         //
        ::VkInstance,                                     //
        ::VkSurfaceKHR,                                   //
        Empty,                                            //
        Empty,                                            //
        internal::begin_surface_adapter,                  //
        internal::close_parented_handle_default_adapter<  //
            ::VkInstance,                                 //
            ::VkSurfaceKHR,                               //
            ::vkDestroySurfaceKHR>>;

using FramebufferBase =                           //
    internal::DefaultParentedHandleResourceBase<  //
        ::VkDevice,                               //
        ::VkFramebuffer,                          //
        ::VkFramebufferCreateInfo,                //
        FramebufferCreateInfo,                    //
        ::vkCreateFramebuffer,                    //
        ::vkDestroyFramebuffer>;

namespace internal {
inline auto create_graphics_pipeline_adapter(
    ::VkDevice device,                           //
    const ::VkGraphicsPipelineCreateInfo& info,  //
    ::VkPipeline& handle) -> ::VkResult {
  return ::vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1,
                                     std::addressof(info), ALLOCATOR,
                                     std::addressof(handle));
}
}  // namespace internal

using GraphicsPipelineBase =                              //
    internal::ParentedHandleBase<                         //
        ::VkDevice,                                       //
        ::VkPipeline,                                     //
        ::VkGraphicsPipelineCreateInfo,                   //
        GraphicsPipelineCreateInfo,                       //
        internal::create_graphics_pipeline_adapter,       //
        internal::close_parented_handle_default_adapter<  //
            ::VkDevice,                                   //
            ::VkPipeline,                                 //
            ::vkDestroyPipeline>>;

//------------------------------------------------------------------------------

DERIVE_FINAL_WITH_CONSTRUCTORS(Instance, InstanceBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(Device, DeviceBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(Semaphore, SemaphoreBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(Fence, FenceBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(Buffer, BufferBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(Queue, QueueBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(DeviceMemory, DeviceMemoryBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(CommandPool, CommandPoolBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(CommandBufferBuilder, CommandBufferBuilderBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(ImageView, ImageViewBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(RenderPass, RenderPassBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(PipelineLayout, PipelineLayoutBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(ShaderModule, ShaderModuleBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(Swapchain, SwapchainBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(Surface, SurfaceBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(Framebuffer, FramebufferBase);
DERIVE_FINAL_WITH_CONSTRUCTORS(GraphicsPipeline, GraphicsPipelineBase);

//------------------------------------------------------------------------------

namespace internal {
inline auto begin_render_pass_command_adapter(
    const ::VkRenderPassBeginInfo& info, ::VkCommandBuffer handle)
    -> ::VkResult {
  ::vkCmdBeginRenderPass(handle, std::addressof(info),
                         VK_SUBPASS_CONTENTS_INLINE);
  return VK_SUCCESS;
}
inline auto end_render_pass_command_adapter(::VkCommandBuffer handle) -> void {
  ::vkCmdEndRenderPass(handle);
}
}  // namespace internal

using RenderPassCommandBuilderBase =                  //
    internal::HandleBase<                             //
        CommandBufferBuilder,                         //
        ::VkRenderPassBeginInfo,                      //
        RenderPassBeginInfo,                          //
        internal::begin_render_pass_command_adapter,  //
        internal::end_render_pass_command_adapter>;

class RenderPassCommandBuilder final : public RenderPassCommandBuilderBase {
  using BaseType = RenderPassCommandBuilderBase;

 public:
  using BaseType::BaseType;

  auto bind_pipeline(::VkPipeline pipeline) -> void {
    ::vkCmdBindPipeline(handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
  }

  auto bind_vertex_buffers(std::uint32_t vertex_buffer_binding,
                           std::span<::VkBuffer> vertex_buffers,
                           std::span<::VkDeviceSize> vertex_buffer_offsets)
      -> void {
    CHECK_PRECONDITION(vertex_buffers.size() == vertex_buffer_offsets.size());
    ::vkCmdBindVertexBuffers(
        handle(),                                           //
        vertex_buffer_binding,                              //
        narrow_cast<std::uint32_t>(vertex_buffers.size()),  //
        vertex_buffers.data(),                              //
        vertex_buffer_offsets.data());
  }

  auto draw(std::uint32_t vertex_count, std::uint32_t instance_count = 1,
            std::uint32_t first_vertex = 0, std::uint32_t first_instance = 0)
      -> void {
    ::vkCmdDraw(handle(), vertex_count, instance_count, first_vertex,
                first_instance);
  }
};

//------------------------------------------------------------------------------

class CommandBufferBlock final {
 public:
  DECLARE_COPY_DELETE(CommandBufferBlock);

  CommandBufferBlock() = default;
  ~CommandBufferBlock() {
    if (block_.size()) {
      ::vkFreeCommandBuffers(device_, pool_, block_.size(), block_.data());
    }
  }

  CommandBufferBlock(CommandBufferBlock&& that) noexcept
      : device_{std::exchange(that.device_, VK_NULL_HANDLE)},
        pool_{std::exchange(that.pool_, VK_NULL_HANDLE)},
        block_{std::move(that.block_)},
        info_{std::move(that.info_)} {}

  auto operator=(CommandBufferBlock&& that) noexcept -> CommandBufferBlock& {
    if (this != &that) {
      if (block_.size()) {
        ::vkFreeCommandBuffers(device_, pool_, block_.size(), block_.data());
      }
      device_ = std::exchange(that.device_, VK_NULL_HANDLE);
      pool_ = std::exchange(that.pool_, VK_NULL_HANDLE);
      block_ = std::move(that.block_);
      info_ = std::move(that.info_);
    }
    return *this;
  }

  explicit CommandBufferBlock(::VkDevice device,
                              const ::VkCommandBufferAllocateInfo& info)
      : device_{device}, info_{info} {
    CHECK_PRECONDITION(device_ != VK_NULL_HANDLE);
    pool_ = info.commandPool;
    block_.resize(info.commandBufferCount);
    ::VkResult result =
        ::vkAllocateCommandBuffers(device_, info_.address(), block_.data());
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

  explicit operator bool() const { return device_ != VK_NULL_HANDLE; }

  auto info() const -> const ::VkCommandBufferAllocateInfo& { return info_(); }

  auto acquire_command_buffers(std::uint32_t next_count) -> void {
    auto curr_count = narrow_cast<std::uint32_t>(block_.size());
    if (curr_count < next_count) {
      block_.resize(next_count);

      ::VkCommandBufferAllocateInfo delta_info{
          .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
          .commandPool = pool_,
          .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
          .commandBufferCount = next_count - curr_count,
      };
      ::VkResult result = ::vkAllocateCommandBuffers(
          device_, std::addressof(delta_info), block_.data() + curr_count);

      CHECK_POSTCONDITION(result == VK_SUCCESS);
    } else if (curr_count > next_count) {
      ::vkFreeCommandBuffers(device_, pool_, curr_count - next_count,
                             block_.data() + next_count);
      block_.resize(next_count);
    }
  }

  auto operator[](std::uint32_t index) const -> ::VkCommandBuffer {
    CHECK_PRECONDITION(index < block_.size());
    return block_[index];
  }

 private:
  ::VkDevice device_ = VK_NULL_HANDLE;
  ::VkCommandPool pool_ = VK_NULL_HANDLE;
  std::vector<::VkCommandBuffer> block_;
  CommandBufferAllocateInfo info_;
};

//------------------------------------------------------------------------------

inline auto has_any_flags(::VkFlags flags, ::VkFlags query) -> bool {
  return (flags & query);
}

inline auto has_all_flags(::VkFlags flags, ::VkFlags query) -> bool {
  return (flags & query) == query;
}

template <typename EnumerationType>
inline auto find_first_flag(::VkFlags flags, std::vector<EnumerationType> query,
                            EnumerationType otherwise) -> EnumerationType {
  auto iter = std::find_if(query.begin(), query.end(),
                           [flags](auto _) { return flags & _; });
  return iter != query.end() ? *iter : otherwise;
}

inline auto has_string_name(const std::vector<const char*>& names,
                            std::string_view target) -> bool {
  return std::any_of(names.begin(), names.end(), [target](const char* name) {
    return static_cast<std::string_view>(name) == target;
  });
}

inline auto has_layer_property(                            //
    const std::span<::VkLayerProperties>& properties,      //
    std::string_view layer_name) -> bool {                 //
  return std::any_of(                                      //
      properties.begin(), properties.end(),                //
      [layer_name](const ::VkLayerProperties& property) {  //
        return static_cast<std::string_view>(property.layerName) == layer_name;
      });
}

inline auto has_extension_property(                                //
    const std::span<::VkExtensionProperties>& properties,          //
    std::string_view extension_name) -> bool {                     //
  return std::any_of(                                              //
      properties.begin(), properties.end(),                        //
      [extension_name](const ::VkExtensionProperties& property) {  //
        return static_cast<std::string_view>(property.extensionName) ==
               extension_name;
      });
}

template <typename TargetFunctionPointer>
inline auto load_instance_function(const char* name, ::VkInstance instance,
                                   Out<TargetFunctionPointer> target) -> void {
  CHECK_PRECONDITION(instance != VK_NULL_HANDLE);
  CHECK_PRECONDITION(name != nullptr);
  *target = reinterpret_cast<TargetFunctionPointer>(
      ::vkGetInstanceProcAddr(instance, name));
  CHECK_POSTCONDITION(*target != nullptr);
}

//------------------------------------------------------------------------------
class DebugMessenger final {
 public:
  DECLARE_COPY_DELETE(DebugMessenger);

  DebugMessenger() = default;
  ~DebugMessenger() {
    if (handle_ != VK_NULL_HANDLE) {
      destroy_(instance_, handle_, ALLOCATOR);
    }
  }

  DebugMessenger(DebugMessenger&& that) noexcept
      : instance_{std::exchange(that.instance_, VK_NULL_HANDLE)},
        handle_{std::exchange(that.handle_, VK_NULL_HANDLE)},
        submit_{std::exchange(that.submit_, nullptr)},
        create_{std::exchange(that.create_, nullptr)},
        destroy_{std::exchange(that.destroy_, nullptr)} {}

  auto operator=(DebugMessenger&& that) noexcept -> DebugMessenger& {
    if (this != &that) {
      if (handle_ != VK_NULL_HANDLE) {
        destroy_(instance_, handle_, ALLOCATOR);
      }
      instance_ = std::exchange(that.instance_, VK_NULL_HANDLE);
      handle_ = std::exchange(that.handle_, VK_NULL_HANDLE);
      submit_ = std::exchange(that.submit_, nullptr);
      create_ = std::exchange(that.create_, nullptr);
      destroy_ = std::exchange(that.destroy_, nullptr);
    }
    return *this;
  }

  explicit DebugMessenger(
      ::VkInstance instance,
      const ::VkDebugUtilsMessengerCreateInfoEXT& create_info)
      : instance_{instance} {
    CHECK_PRECONDITION(instance_ != VK_NULL_HANDLE);

    vk::load_instance_function(CREATE_FUNCTION_NAME, instance_, Out(create_));
    vk::load_instance_function(DESTROY_FUNCTION_NAME, instance_, Out(destroy_));
    vk::load_instance_function(SUBMIT_FUNCTION_NAME, instance_, Out(submit_));

    ::VkResult result = create_(instance_, std::addressof(create_info),
                                ALLOCATOR, std::addressof(handle_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);
    CHECK_POSTCONDITION(handle_ != VK_NULL_HANDLE);
  }

  explicit operator bool() const {
    return instance_ != VK_NULL_HANDLE && handle_ != VK_NULL_HANDLE;
  }

 private:
  static constexpr const char* SUBMIT_FUNCTION_NAME =
      "vkSubmitDebugUtilsMessageEXT";
  static constexpr const char* CREATE_FUNCTION_NAME =
      "vkCreateDebugUtilsMessengerEXT";
  static constexpr const char* DESTROY_FUNCTION_NAME =
      "vkDestroyDebugUtilsMessengerEXT";

  ::VkInstance instance_ = VK_NULL_HANDLE;
  ::VkDebugUtilsMessengerEXT handle_ = VK_NULL_HANDLE;
  ::PFN_vkSubmitDebugUtilsMessageEXT submit_ = nullptr;
  ::PFN_vkCreateDebugUtilsMessengerEXT create_ = nullptr;
  ::PFN_vkDestroyDebugUtilsMessengerEXT destroy_ = nullptr;
};

//------------------------------------------------------------------------------

inline auto convert_to_string(::VkDebugUtilsMessageSeverityFlagBitsEXT _)
    -> std::string_view {
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
  // Drivers and extensions can report values newer than this list.
  return "UNKNOWN";
}

inline auto convert_to_string(::VkPhysicalDeviceType _) -> std::string_view {
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
  // Drivers and extensions can report values newer than this list.
  return "UNKNOWN";
}

inline auto convert_to_string(::VkQueueFlagBits _) -> std::string_view {
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
  // Drivers and extensions can report values newer than this list.
  return "UNKNOWN";
}

inline auto convert_to_string(::VkQueueFlags flags) -> std::string {
  std::stringstream stream;
  for (auto bit :
       {VK_QUEUE_GRAPHICS_BIT, VK_QUEUE_COMPUTE_BIT, VK_QUEUE_TRANSFER_BIT,
        VK_QUEUE_SPARSE_BINDING_BIT, VK_QUEUE_PROTECTED_BIT,
        VK_QUEUE_VIDEO_DECODE_BIT_KHR, VK_QUEUE_VIDEO_ENCODE_BIT_KHR}) {
    if (flags & bit) {
      stream << convert_to_string(static_cast<::VkQueueFlagBits>(flags & bit))
             << ", ";
    }
  }
  return std::move(stream).str();
}

inline auto convert_to_string(::VkFormat _) -> std::string_view {
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
  // Drivers and extensions can report values newer than this list.
  return "UNKNOWN";
}

inline auto convert_to_string(::VkPresentModeKHR _) -> std::string_view {
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
  // Drivers and extensions can report values newer than this list.
  return "UNKNOWN";
}

}  // namespace volcano::vk
