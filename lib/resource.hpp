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
#include "vk/resource.hpp"

namespace volcano {
namespace impl {

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
  ~Buffer() = default;

 private:
  friend class Device;

  explicit Buffer(::VkDevice device,          //
                  ::VkDeviceSize byte_count,  //
                  ::VkBufferUsageFlags buffer_usage)
      : device_{device} {
    buffer_ = vk::Buffer{device_, ::VkBufferCreateInfo{
                                      .size = byte_count,
                                      .usage = buffer_usage,
                                      .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                  }};
    memory_requirements_ = vk::MemoryRequirements{device_, buffer_};
  }

  ::VkDevice device_ = VK_NULL_HANDLE;
  vk::Buffer buffer_;
  vk::MemoryRequirements memory_requirements_;
};

//------------------------------------------------------------------------------
class DeviceMemory final {
 public:
  DECLARE_COPY_DELETE(DeviceMemory);
  DECLARE_MOVE_DEFAULT(DeviceMemory);

  DeviceMemory() = delete;
  ~DeviceMemory() = default;

  void CopyInitialize(std::span<const std::byte> data) {
    CHECK_PRECONDITION(data.size() <= memory_.info().allocationSize);
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
    memory_ =
        vk::DeviceMemory{device_, ::VkMemoryAllocateInfo{
                                      .allocationSize = required_byte_count,
                                      .memoryTypeIndex = memory_type_index,
                                  }};

    ::VkResult result = ::vkBindBufferMemory(device_, target_buffer, memory_,
                                             required_byte_offset);
    CHECK_POSTCONDITION(result == VK_SUCCESS);

    void* host_pointer = nullptr;
    ::VkMemoryMapFlags flags = 0;
    result = ::vkMapMemory(device_, memory_, required_byte_offset,
                           VK_WHOLE_SIZE, flags, std::addressof(host_pointer));
    CHECK_POSTCONDITION(result == VK_SUCCESS);

    host_bytes_ = reinterpret_cast<std::byte*>(host_pointer);
  }

  ::VkDevice device_ = VK_NULL_HANDLE;
  vk::DeviceMemory memory_;
  std::byte* host_bytes_ = nullptr;
};

//------------------------------------------------------------------------------
class CommandPool final {
 public:
  DECLARE_COPY_DELETE(CommandPool);
  DECLARE_MOVE_DEFAULT(CommandPool);

  CommandPool() = delete;
  ~CommandPool() = default;

 private:
  friend class Device;

  explicit CommandPool(::VkDevice device, std::uint32_t queue_family_index)
      : device_{device} {
    command_pool_ = vk::CommandPool{
        device_,
        ::VkCommandPoolCreateInfo{.queueFamilyIndex = queue_family_index}};
  }

  ::VkDevice device_ = VK_NULL_HANDLE;
  vk::CommandPool command_pool_;
};

//------------------------------------------------------------------------------
class ImageView final {
 public:
  DECLARE_COPY_DELETE(ImageView);
  DECLARE_MOVE_DEFAULT(ImageView);

  ImageView() = delete;
  ~ImageView() = default;

 private:
  friend class Device;

  explicit ImageView(::VkDevice device, ::VkImage image, ::VkFormat format)
      : device_{device} {
    ::VkImageViewCreateInfo create_info{};

    create_info.image = image;
    create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    create_info.format = format,
    create_info.components = {VK_COMPONENT_SWIZZLE_IDENTITY,  //
                              VK_COMPONENT_SWIZZLE_IDENTITY,  //
                              VK_COMPONENT_SWIZZLE_IDENTITY,  //
                              VK_COMPONENT_SWIZZLE_IDENTITY};
    create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    create_info.subresourceRange.baseMipLevel = 0;
    create_info.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    create_info.subresourceRange.baseArrayLayer = 0;
    create_info.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    image_view_ = vk::ImageView{device_, create_info};
  }

  ::VkDevice device_ = VK_NULL_HANDLE;
  vk::ImageView image_view_;
};

//------------------------------------------------------------------------------
class RenderPass final {
 public:
  DECLARE_COPY_DELETE(RenderPass);
  DECLARE_MOVE_DEFAULT(RenderPass);

  RenderPass() = delete;
  ~RenderPass() = default;

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

    render_pass_ = vk::RenderPass{
        device_, ::VkRenderPassCreateInfo{
                     .attachmentCount = color_attachment.size(),
                     .pAttachments = color_attachment.data(),
                     .subpassCount = subpass_description.size(),
                     .pSubpasses = subpass_description.data(),
                     .dependencyCount = subpass_dependencies.size(),
                     .pDependencies = subpass_dependencies.data(),
                 }};
  }

  ::VkDevice device_ = VK_NULL_HANDLE;
  vk::RenderPass render_pass_;
};

//------------------------------------------------------------------------------
class PipelineLayout final {
 public:
  DECLARE_COPY_DELETE(PipelineLayout);
  DECLARE_MOVE_DEFAULT(PipelineLayout);

  PipelineLayout() = delete;
  ~PipelineLayout() = default;

 private:
  friend class Device;

  explicit PipelineLayout(::VkDevice device) : device_{device} {
    pipeline_layout_ =
        vk::PipelineLayout{device_, ::VkPipelineLayoutCreateInfo{}};
  }

  ::VkDevice device_ = VK_NULL_HANDLE;
  vk::PipelineLayout pipeline_layout_;
};

//------------------------------------------------------------------------------
class ShaderModule final {
 public:
  DECLARE_COPY_DELETE(ShaderModule);
  DECLARE_MOVE_DEFAULT(ShaderModule);

  ShaderModule() = delete;
  ~ShaderModule() = default;

 private:
  friend class Device;

  explicit ShaderModule(::VkDevice device,
                        const std::vector<std::uint32_t>& shader_spirv_bin)
      : device_{device} {
    ::VkShaderModuleCreateInfo create_info{};
    create_info.pCode = shader_spirv_bin.data();
    create_info.codeSize =
        shader_spirv_bin.size() * sizeof(std::uint32_t);  // Byte count.

    shader_module_ = vk::ShaderModule{device_, create_info};
  }

  ::VkDevice device_ = VK_NULL_HANDLE;
  vk::ShaderModule shader_module_;
};

//------------------------------------------------------------------------------
class Swapchain final {
 public:
  DECLARE_COPY_DELETE(Swapchain);
  DECLARE_MOVE_DEFAULT(Swapchain);

  Swapchain() = delete;
  ~Swapchain() = default;

  std::span<::VkImage> Images() { return {swapchain_images_}; }

 private:
  friend class Device;

  explicit Swapchain(::VkDevice device,                                       //
                     std::vector<std::uint32_t> queue_families,               //
                     ::VkSurfaceKHR surface,                                  //
                     const ::VkSurfaceCapabilitiesKHR& surface_capabilities,  //
                     const ::VkSurfaceFormatKHR& surface_format,              //
                     ::VkPresentModeKHR surface_present_mode,                 //
                     ::VkSwapchainKHR previous_swapchain)
      : device_{device},
        queue_families_{std::move(queue_families)},
        surface_{surface},
        surface_capabilities_{surface_capabilities},
        surface_format_{surface_format} {
    VkCompositeAlphaFlagBitsKHR composite_alpha =
        vk::FindFirstFlag(surface_capabilities_.supportedCompositeAlpha,
                          {VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                           VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                           VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
                           VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR},
                          static_cast<::VkCompositeAlphaFlagBitsKHR>(-1));
    CHECK_INVARIANT(composite_alpha != -1);

    swapchain_ = vk::Swapchain{
        device_, ::VkSwapchainCreateInfoKHR{
                     .surface = surface_,
                     .minImageCount = surface_capabilities_.minImageCount + 1,
                     .imageFormat = surface_format_.format,
                     .imageColorSpace = surface_format_.colorSpace,
                     .imageExtent = surface_capabilities_.currentExtent,
                     .imageArrayLayers = 1,  // Non-stereoscopic.
                     .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                     .imageSharingMode = queue_families_.size() > 1
                                             ? VK_SHARING_MODE_CONCURRENT
                                             : VK_SHARING_MODE_EXCLUSIVE,
                     .queueFamilyIndexCount = queue_families_.size(),
                     .pQueueFamilyIndices = queue_families_.data(),
                     .preTransform = surface_capabilities_.currentTransform,
                     .compositeAlpha = composite_alpha,
                     .presentMode = surface_present_mode,
                     .clipped = VK_TRUE,
                     .oldSwapchain = previous_swapchain,
                 }};

    swapchain_images_ = vk::SwapchainImages{device_, swapchain_};
  }

  ::VkDevice device_ = VK_NULL_HANDLE;
  vk::Swapchain swapchain_;
  vk::SwapchainImages swapchain_images_;

  std::vector<std::uint32_t> queue_families_;
  ::VkSurfaceKHR surface_ = VK_NULL_HANDLE;
  ::VkSurfaceCapabilitiesKHR surface_capabilities_;
  ::VkSurfaceFormatKHR surface_format_;
};

//------------------------------------------------------------------------------
class Device final {
 public:
  DECLARE_COPY_DELETE(Device);
  DECLARE_MOVE_DEFAULT(Device);

  Device() = delete;
  ~Device() = default;

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
      if ((buffer.memory_requirements_().memoryTypeBits &
           (1u << memory_type_index)) &&
          vk::HasAllFlags(
              phys_device_memory_properties_.memoryTypes[memory_type_index]
                  .propertyFlags,
              required_memory_flags)) {
        found = true;
        break;
      }
    }
    CHECK_POSTCONDITION(found);

    ::VkDeviceSize byte_offset = 0;
    return DeviceMemory{device_, byte_offset,
                        buffer.memory_requirements_().size, memory_type_index,
                        buffer.buffer_};
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

  std::vector<ImageView> CreateImageViews(std::span<::VkImage> images,
                                          ::VkFormat format) {
    std::vector<ImageView> result;
    for (auto&& image : images) {
      result.push_back(ImageView{device_, image, format});
    }
    return result;
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
      device_queue_infos_.push_back(::VkDeviceQueueCreateInfo{
          .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO  //
      });
      device_queue_infos_.back().queueFamilyIndex = queue_family_i;
      device_queue_infos_.back().queueCount = queue_priority.size();
      device_queue_infos_.back().pQueuePriorities = queue_priority.data();
    }

    ::VkDeviceCreateInfo create_info{};
    create_info.queueCreateInfoCount = device_queue_infos_.size();
    create_info.pQueueCreateInfos = device_queue_infos_.data();
    create_info.enabledExtensionCount = device_extensions_.size();
    create_info.ppEnabledExtensionNames = device_extensions_.data();
    create_info.pEnabledFeatures = std::addressof(phys_device_features_);

    device_ = vk::Device{phys_device, create_info};

    ::VkResult result = ::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
        phys_device, surface_, std::addressof(surface_capabilities_));
    CHECK_POSTCONDITION(result == VK_SUCCESS);

    vk::MaybeEnumerateProperties(
        std::bind_front(::vkGetPhysicalDeviceSurfaceFormatsKHR, phys_device,
                        surface_),
        InOut(surface_formats_));

    std::print("Surface Formats: \n");
    for (auto&& surface_format : surface_formats_) {
      std::print(" :: {}\n", vk::ConvertToString(surface_format.format));
    }

    vk::MaybeEnumerateProperties(
        std::bind_front(::vkGetPhysicalDeviceSurfacePresentModesKHR,
                        phys_device, surface_),
        InOut(surface_present_modes_));

    std::print("Surface Present Modes: \n");
    for (auto&& surface_present_mode : surface_present_modes_) {
      std::print(" .. {}\n", vk::ConvertToString(surface_present_mode));
    }
  }

  vk::Device device_;

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
    if (instance_ && destroy_debug_messenger_ &&
        debug_messenger_ != VK_NULL_HANDLE) {
      destroy_debug_messenger_(instance_, debug_messenger_, vk::ALLOCATOR);
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
    instance_info_().pApplicationInfo = app_info;

    if (debug_level != DebugLevel::NONE &&
        vk::HasStringName(instance_extensions_, impl::DEBUG_EXTENSION_NAME)) {
      instance_info_().pNext = debug_messenger_info_.address();

      debug_messenger_info_().messageSeverity =
          ConvertToDebugSeverity(debug_level);
      debug_messenger_info_().messageType =
          VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      debug_messenger_info_().pfnUserCallback = DebugMessengerCallback;
    }

    instance_info_().enabledLayerCount = instance_layers_.size();
    instance_info_().ppEnabledLayerNames = instance_layers_.data();
    instance_info_().enabledExtensionCount = instance_extensions_.size();
    instance_info_().ppEnabledExtensionNames = instance_extensions_.data();

    std::print("Requested Layers: \n");
    for (auto&& layer : instance_layers_) {
      std::print(" == {}\n", layer);
    }

    std::print("Requested Extensions: \n");
    for (auto&& extension : instance_extensions_) {
      std::print(" -- {}\n", extension);
    }

    instance_ = vk::Instance{instance_info_()};

    if (debug_level != DebugLevel::NONE &&
        vk::HasStringName(instance_extensions_, impl::DEBUG_EXTENSION_NAME)) {
      vk::LoadInstanceFunction(impl::DEBUG_CREATE_FUNCTION_NAME, instance_,
                               Out(create_debug_messenger_));
      vk::LoadInstanceFunction(impl::DEBUG_DESTROY_FUNCTION_NAME, instance_,
                               Out(destroy_debug_messenger_));
      vk::LoadInstanceFunction(impl::DEBUG_SUBMIT_FUNCTION_NAME, instance_,
                               Out(submit_debug_message_));

      ::VkResult result = create_debug_messenger_(
          instance_, debug_messenger_info_.address(), vk::ALLOCATOR,
          std::addressof(debug_messenger_));
      CHECK_POSTCONDITION(result == VK_SUCCESS);
      CHECK_POSTCONDITION(debug_messenger_ != VK_NULL_HANDLE);
    }

    phys_devices_ = vk::PhysicalDevices{instance_};

    std::print("Physical Devices: \n");
    for (auto&& phys_device : phys_devices_()) {
      ::vkGetPhysicalDeviceProperties(
          phys_device, std::addressof(phys_device_properties_[phys_device]));
      std::print(
          " ** {} [{}]\n", phys_device_properties_[phys_device].deviceName,
          vk::ConvertToString(phys_device_properties_[phys_device].deviceType));

      ::vkGetPhysicalDeviceMemoryProperties(
          phys_device,
          std::addressof(phys_device_memory_properties_[phys_device]));

      ::vkGetPhysicalDeviceFeatures(
          phys_device, std::addressof(phys_device_features_[phys_device]));

      vk::MaybeEnumerateProperties(
          std::bind_front(::vkGetPhysicalDeviceQueueFamilyProperties,
                          phys_device),
          InOut(phys_device_queue_family_properties_[phys_device]));

      std::print("Queue Family Flags: \n");
      for (auto&& property :
           phys_device_queue_family_properties_[phys_device]) {
        std::print(" .. [{}] {}\n", property.queueCount,
                   vk::ConvertToString(property.queueFlags));
      }

      vk::MaybeEnumerateProperties(
          std::bind_front(::vkEnumerateDeviceExtensionProperties, phys_device,
                          nullptr),
          InOut(supported_device_extension_properties_[phys_device]));

      std::print("Supported Device Extensions: \n");
      for (auto&& property :
           supported_device_extension_properties_[phys_device]) {
        std::print(" -- {}\n", property.extensionName);
      }

      CHECK_INVARIANT(vk::HasExtensionProperty(
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

    for (auto&& phys_device : phys_devices_()) {
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

  vk::Instance instance_;
  ::VkDebugUtilsMessengerEXT debug_messenger_ = VK_NULL_HANDLE;

  ::PFN_vkSubmitDebugUtilsMessageEXT submit_debug_message_ = nullptr;
  ::PFN_vkCreateDebugUtilsMessengerEXT create_debug_messenger_ = nullptr;
  ::PFN_vkDestroyDebugUtilsMessengerEXT destroy_debug_messenger_ = nullptr;

  vk::InstanceCreateInfo instance_info_;
  vk::DebugUtilsMessengerCreateInfo debug_messenger_info_;

  std::vector<const char*> instance_layers_;
  std::vector<const char*> instance_extensions_;

  vk::PhysicalDevices phys_devices_;

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
    std::print("[{}] <{}> {}\n", vk::ConvertToString(message_severity),
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
    for (auto&& _ : supported_layers_()) {
      supported_extensions_[_.layerName] =
          vk::InstanceExtensionProperties{_.layerName};
    }

    for (auto&& layer : supported_layers_()) {
      std::print("Supported Instance Layer: {}\n", layer.layerName);
      for (auto&& extension : supported_extensions_[layer.layerName]()) {
        std::print("  Supported Instance Extension: {}\n",
                   extension.extensionName);
      }
    }

    application_info_().pApplicationName = name_.c_str();
    application_info_().applicationVersion = version;
    application_info_().apiVersion = VK_API_VERSION_1_3;
  }

  Instance CreateInstance(std::span<const char*> requested_layers = {},
                          std::span<const char*> requested_extensions = {},
                          DebugLevel debug_level = DebugLevel::NONE) {
    std::vector<const char*> layers{requested_layers.begin(),
                                    requested_layers.end()};
    std::vector<const char*> extensions{requested_extensions.begin(),
                                        requested_extensions.end()};

    if (vk::HasLayerProperty(supported_layers_, impl::VALIDATION_LAYER_NAME)) {
      layers.push_back(impl::VALIDATION_LAYER_NAME);
    }

    if (debug_level != DebugLevel::NONE) {
      if (vk::HasExtensionProperty(
              supported_extensions_[impl::VALIDATION_LAYER_NAME],
              impl::DEBUG_EXTENSION_NAME)) {
        extensions.push_back(impl::DEBUG_EXTENSION_NAME);
      } else {
        std::cerr << "Missing debug extension: " << impl::DEBUG_EXTENSION_NAME;
      }
    }

    return Instance{application_info_.address(),  //
                    std::move(layers),            //
                    std::move(extensions),        //
                    debug_level};
  }

 private:
  vk::ApplicationInfo application_info_;
  vk::InstanceLayerProperties supported_layers_;
  std::map<std::string, vk::InstanceExtensionProperties> supported_extensions_;
  std::string name_;
};

}  // namespace volcano
