#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <chrono>
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

  std::uint32_t family_index() const { return index_.family_index; }

 private:
  vk::Queue queue_;
  vk::QueueIndex index_;

  friend class Device;

  explicit Queue(::VkDevice device,                 //
                 std::uint32_t queue_family_index,  //
                 std::uint32_t queue_index) {
    CHECK_PRECONDITION(device != VK_NULL_HANDLE);
    index_ = vk::QueueIndex{queue_family_index, queue_index};
    queue_ = vk::Queue{device, index_};
  }
};

//------------------------------------------------------------------------------
class Semaphore final {
 public:
  DECLARE_COPY_DELETE(Semaphore);
  DECLARE_MOVE_DEFAULT(Semaphore);

  Semaphore() = delete;
  ~Semaphore() = default;

  operator ::VkSemaphore() const { return semaphore_; }

 private:
  friend class Device;

  explicit Semaphore(::VkDevice device) {
    semaphore_ = vk::Semaphore{device, ::VkSemaphoreCreateInfo{}};
  }

  vk::Semaphore semaphore_;
};

//------------------------------------------------------------------------------
class Fence final {
 public:
  DECLARE_COPY_DELETE(Fence);
  DECLARE_MOVE_DEFAULT(Fence);

  Fence() = delete;
  ~Fence() = default;

  operator ::VkFence() const { return fence_; }

  void wait(std::chrono::nanoseconds timeout) {
    ::VkResult result =
        ::vkWaitForFences(fence_.parent(), 1, std::addressof(fence_.handle()),
                          VK_TRUE, timeout.count());
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

  void reset() {
    ::VkResult result =
        ::vkResetFences(fence_.parent(), 1, std::addressof(fence_.handle()));
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

 private:
  friend class Device;

  explicit Fence(::VkDevice device, ::VkFenceCreateFlags flags) {
    fence_ = vk::Fence{device, ::VkFenceCreateInfo{.flags = flags}};
  }

  vk::Fence fence_;
};

//------------------------------------------------------------------------------
class Buffer final {
 public:
  DECLARE_COPY_DELETE(Buffer);
  DECLARE_MOVE_DEFAULT(Buffer);

  Buffer() = delete;
  ~Buffer() = default;

  operator ::VkBuffer() const { return buffer_; }

 private:
  friend class Device;

  explicit Buffer(::VkDevice device,          //
                  ::VkDeviceSize byte_count,  //
                  ::VkBufferUsageFlags buffer_usage) {
    buffer_ = vk::Buffer{device, ::VkBufferCreateInfo{
                                     .size = byte_count,
                                     .usage = buffer_usage,
                                     .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
                                 }};
    memory_requirements_ = vk::MemoryRequirements{device, buffer_};
  }

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

  void copy_initialize(std::span<const std::byte> data) {
    CHECK_PRECONDITION(data.size() <= memory_.info().allocationSize);
    CHECK_PRECONDITION(host_bytes_);

    std::copy(data.begin(), data.end(), host_bytes_);
    ::VkDevice device = memory_.parent();
    ::vkUnmapMemory(device, memory_);

    host_bytes_ = nullptr;
  }

 private:
  friend class Device;

  explicit DeviceMemory(::VkDevice device,                    //
                        ::VkDeviceSize required_byte_offset,  //
                        ::VkDeviceSize required_byte_count,   //
                        std::uint32_t memory_type_index,      //
                        ::VkBuffer target_buffer) {
    memory_ =
        vk::DeviceMemory{device, ::VkMemoryAllocateInfo{
                                     .allocationSize = required_byte_count,
                                     .memoryTypeIndex = memory_type_index,
                                 }};

    ::VkResult result = ::vkBindBufferMemory(device, target_buffer, memory_,
                                             required_byte_offset);
    CHECK_POSTCONDITION(result == VK_SUCCESS);

    void* host_pointer = nullptr;
    ::VkMemoryMapFlags flags = 0;
    result = ::vkMapMemory(device, memory_, required_byte_offset, VK_WHOLE_SIZE,
                           flags, std::addressof(host_pointer));
    CHECK_POSTCONDITION(result == VK_SUCCESS);

    host_bytes_ = reinterpret_cast<std::byte*>(host_pointer);
  }

  vk::DeviceMemory memory_;
  std::byte* host_bytes_ = nullptr;
};

//------------------------------------------------------------------------------
class RenderPassCommandBuffer final {
 public:
  DECLARE_COPY_DELETE(RenderPassCommandBuffer);
  DECLARE_MOVE_DEFAULT(RenderPassCommandBuffer);

  RenderPassCommandBuffer() = delete;
  ~RenderPassCommandBuffer() = default;

  void bind(::VkPipeline pipeline) { command_.bind_pipeline(pipeline); }

  void bind(std::uint32_t vertex_buffer_binding,
            std::span<::VkBuffer> vertex_buffers,
            std::span<::VkDeviceSize> vertex_buffer_offsets) {
    command_.bind_vertex_buffers(vertex_buffer_binding, vertex_buffers,
                                 vertex_buffer_offsets);
  }

  void draw(std::uint32_t vertex_count) { command_.draw(vertex_count); }

 private:
  friend class CommandBufferBlock;

  explicit RenderPassCommandBuffer(::VkCommandBuffer command_buffer,
                                   ::VkRenderPass render_pass,
                                   ::VkFramebuffer framebuffer,
                                   ::VkExtent2D framebuffer_extent) {
    static std::array<::VkClearValue, 1>  //
        clear_values{::VkClearValue{.color = {
                                        .float32 = {0.1f, 0.1f, 0.1f, 1.0f},
                                    }}};

    command_ = vk::RenderPassCommandBuffer{
        vk::CommandBuffer{
            command_buffer,
            ::VkCommandBufferBeginInfo{
                .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
            }},
        ::VkRenderPassBeginInfo{
            .renderPass = render_pass,
            .framebuffer = framebuffer,
            .renderArea = {.offset = {.x = 0, .y = 0},
                           .extent = framebuffer_extent},
            .clearValueCount = narrow_cast<std::uint32_t>(clear_values.size()),
            .pClearValues = clear_values.data(),
        }};
  }

  vk::RenderPassCommandBuffer command_;
};

//------------------------------------------------------------------------------
class CommandBufferBlock final {
 public:
  DECLARE_COPY_DELETE(CommandBufferBlock);
  DECLARE_MOVE_DEFAULT(CommandBufferBlock);

  CommandBufferBlock() = delete;
  ~CommandBufferBlock() = default;

  void acquire_command_buffers(std::uint32_t count) {
    command_buffers_.acquire_command_buffers(count);
  }

  RenderPassCommandBuffer create_render_pass_command_buffer(
      std::uint32_t command_buffer_index,  //
      ::VkRenderPass render_pass,          //
      ::VkFramebuffer framebuffer,         //
      ::VkExtent2D framebuffer_extent) {
    return RenderPassCommandBuffer{command_buffers_[command_buffer_index],  //
                                   render_pass,                             //
                                   framebuffer,                             //
                                   framebuffer_extent};
  }

 private:
  friend class Device;

  explicit CommandBufferBlock(::VkDevice device, ::VkCommandPool pool,
                              std::uint32_t count) {
    command_buffers_ = vk::CommandBufferBlock{
        device, ::VkCommandBufferAllocateInfo{
                    .commandPool = pool,
                    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
                    .commandBufferCount = count,
                }};
  }

  vk::CommandBufferBlock command_buffers_;
};

//------------------------------------------------------------------------------
class CommandPool final {
 public:
  DECLARE_COPY_DELETE(CommandPool);
  DECLARE_MOVE_DEFAULT(CommandPool);

  CommandPool() = delete;
  ~CommandPool() = default;

  operator ::VkCommandPool() const { return command_pool_; }

  void reset() {
    ::VkResult result =
        ::vkResetCommandPool(command_pool_.parent(), command_pool_.handle(), 0);
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

 private:
  friend class Device;

  explicit CommandPool(::VkDevice device, std::uint32_t queue_family_index) {
    command_pool_ =
        vk::CommandPool{device, ::VkCommandPoolCreateInfo{
                                    .queueFamilyIndex = queue_family_index,
                                }};
  }

  vk::CommandPool command_pool_;
};

//------------------------------------------------------------------------------
class ImageView final {
 public:
  DECLARE_COPY_DELETE(ImageView);
  DECLARE_MOVE_DEFAULT(ImageView);

  ImageView() = delete;
  ~ImageView() = default;

  operator ::VkImageView() const { return image_view_; }

 private:
  friend class Swapchain;

  explicit ImageView(::VkDevice device, ::VkImage image, ::VkFormat format) {
    image_view_ = vk::ImageView{
        device, ::VkImageViewCreateInfo{
                    .image = image,
                    .viewType = VK_IMAGE_VIEW_TYPE_2D,
                    .format = format,
                    .components = {VK_COMPONENT_SWIZZLE_IDENTITY,  //
                                   VK_COMPONENT_SWIZZLE_IDENTITY,  //
                                   VK_COMPONENT_SWIZZLE_IDENTITY,  //
                                   VK_COMPONENT_SWIZZLE_IDENTITY},
                    .subresourceRange = {
                        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                        .baseMipLevel = 0,
                        .levelCount = VK_REMAINING_MIP_LEVELS,
                        .baseArrayLayer = 0,
                        .layerCount = VK_REMAINING_ARRAY_LAYERS,
                    }}};
  }

  vk::ImageView image_view_;
};

//------------------------------------------------------------------------------
class Framebuffer final {
 public:
  DECLARE_COPY_DELETE(Framebuffer);
  DECLARE_MOVE_DEFAULT(Framebuffer);

  Framebuffer() = delete;
  ~Framebuffer() = default;

  operator ::VkFramebuffer() const { return framebuffer_; }
  ::VkExtent2D extent() const { return extent_; }

 private:
  friend class Device;

  explicit Framebuffer(::VkDevice device,            //
                       ::VkRenderPass render_pass,   //
                       ::VkImageView image_view,     //
                       ::VkExtent2D surface_extent,  //
                       std::uint32_t surface_layers = 1)
      : image_view_{image_view},  //
        extent_{surface_extent} {
    framebuffer_ =
        vk::Framebuffer{device, ::VkFramebufferCreateInfo{
                                    .renderPass = render_pass,
                                    .attachmentCount = 1,
                                    .pAttachments = std::addressof(image_view_),
                                    .width = surface_extent.width,
                                    .height = surface_extent.height,
                                    .layers = surface_layers,
                                }};
  }

  vk::Framebuffer framebuffer_;
  ::VkImageView image_view_ = VK_NULL_HANDLE;
  ::VkExtent2D extent_{};
};

//------------------------------------------------------------------------------
class RenderPass final {
 public:
  DECLARE_COPY_DELETE(RenderPass);
  DECLARE_MOVE_DEFAULT(RenderPass);

  RenderPass() = delete;
  ~RenderPass() = default;

  operator ::VkRenderPass() const { return render_pass_; }

 private:
  friend class Device;

  explicit RenderPass(::VkDevice device, ::VkFormat format) {
    static std::array<::VkAttachmentDescription, 1>  //
        color_attachment{
            ::VkAttachmentDescription{
                .samples = VK_SAMPLE_COUNT_1_BIT,
                .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
                .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
                .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
                .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
                .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
            },
        };

    static const std::array<const ::VkAttachmentReference, 1>  //
        color_reference{
            ::VkAttachmentReference{
                .attachment =
                    0,  // Index into `VkRenderPassCreateInfo::pAttachments`.
                .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            },
        };

    static const std::array<const ::VkSubpassDescription, 1>  //
        subpass_description{
            ::VkSubpassDescription{
                .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
                .inputAttachmentCount = 0,
                .pInputAttachments = nullptr,
                .colorAttachmentCount =
                    narrow_cast<std::uint32_t>(color_reference.size()),
                .pColorAttachments = color_reference.data(),
            },
        };

    static const std::array<const ::VkSubpassDependency, 2>
        subpass_dependencies{
            // Source.
            ::VkSubpassDependency{
                .srcSubpass = VK_SUBPASS_EXTERNAL,
                .dstSubpass =
                    0,  // Index into `VkRenderPassCreateInfo::pSubpasses`.
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .srcAccessMask = 0,
                .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            },
            // Destination.
            ::VkSubpassDependency{
                .srcSubpass =
                    0,  // Index into `VkRenderPassCreateInfo::pSubpasses`.
                .dstSubpass = VK_SUBPASS_EXTERNAL,
                .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                .dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                .srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                .dstAccessMask = 0,
                .dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT,
            }};

    color_attachment.front().format = format;

    render_pass_ = vk::RenderPass{
        device, ::VkRenderPassCreateInfo{
                    .attachmentCount =
                        narrow_cast<std::uint32_t>(color_attachment.size()),
                    .pAttachments = color_attachment.data(),
                    .subpassCount =
                        narrow_cast<std::uint32_t>(subpass_description.size()),
                    .pSubpasses = subpass_description.data(),
                    .dependencyCount =
                        narrow_cast<std::uint32_t>(subpass_dependencies.size()),
                    .pDependencies = subpass_dependencies.data(),
                }};
  }

  vk::RenderPass render_pass_;
};

//------------------------------------------------------------------------------
class PipelineLayout final {
 public:
  DECLARE_COPY_DELETE(PipelineLayout);
  DECLARE_MOVE_DEFAULT(PipelineLayout);

  PipelineLayout() = delete;
  ~PipelineLayout() = default;

  operator ::VkPipelineLayout() const { return pipeline_layout_; }

 private:
  friend class Device;

  explicit PipelineLayout(::VkDevice device) {
    pipeline_layout_ =
        vk::PipelineLayout{device, ::VkPipelineLayoutCreateInfo{}};
  }

  vk::PipelineLayout pipeline_layout_;
};

//------------------------------------------------------------------------------
class GraphicsPipeline final {
 public:
  DECLARE_COPY_DELETE(GraphicsPipeline);
  DECLARE_MOVE_DEFAULT(GraphicsPipeline);

  GraphicsPipeline() = delete;
  ~GraphicsPipeline() = default;

  operator ::VkPipeline() const { return pipeline_; }

 private:
  friend class Device;

  explicit GraphicsPipeline(::VkDevice device,  //
                            ::VkShaderModule vertex_shader,
                            ::VkShaderModule fragment_shader,
                            ::VkPipelineLayout pipeline_layout,
                            ::VkRenderPass render_pass,
                            ::VkExtent2D surface_extent) {
    std::array<::VkPipelineShaderStageCreateInfo, 2> shader_stage_info{
        ::VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertex_shader,
            .pName = "main",  // Entry point for vertex shader in module.
        },
        ::VkPipelineShaderStageCreateInfo{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragment_shader,
            .pName = "main",  // Entry point for fragment shader in module.
        },
    };

    std::array<::VkVertexInputBindingDescription, 1>  //
        vertex_input_binding_desc{
            ::VkVertexInputBindingDescription{
                .binding = 0,                 // Declare "binding 0".
                .stride = 5 * sizeof(float),  // sizeof(Vertex2D_ColorF_pack)
                .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
            },
        };

    std::array<::VkVertexInputAttributeDescription, 2>  //
        vertex_input_attribute_desc{
            ::VkVertexInputAttributeDescription{
                .location = 0,  // Declare "location 0".
                .binding = 0,   // Wrt. "binding 0".
                .format = VK_FORMAT_R32G32_SFLOAT,
                // offsetof(Vertex2D_ColorF_pack, position)
                .offset = 0 * sizeof(float),
            },
            ::VkVertexInputAttributeDescription{
                .location = 1,  // Declare "location 1".
                .binding = 0,   // Wrt. "binding 0".
                .format = VK_FORMAT_R32G32_SFLOAT,
                // offsetof(Vertex2D_ColorF_pack, color)
                .offset = 2 * sizeof(float),
            },
        };

    ::VkPipelineVertexInputStateCreateInfo vertex_input_state_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount =
            narrow_cast<std::uint32_t>(vertex_input_binding_desc.size()),
        .pVertexBindingDescriptions = vertex_input_binding_desc.data(),
        .vertexAttributeDescriptionCount =
            narrow_cast<std::uint32_t>(vertex_input_attribute_desc.size()),
        .pVertexAttributeDescriptions = vertex_input_attribute_desc.data(),
    };

    static ::VkPipelineInputAssemblyStateCreateInfo input_assembly_state_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    std::array<::VkViewport, 1> viewports{
        ::VkViewport{
            .x = 0.f,
            .y = 0.f,
            .width = static_cast<float>(surface_extent.width),
            .height = static_cast<float>(surface_extent.height),
            .minDepth = 0.f,
            .maxDepth = 1.f,
        },
    };

    std::array<::VkRect2D, 1> scissors{
        ::VkRect2D{.offset = {.x = 0, .y = 0}, .extent = surface_extent},
    };

    ::VkPipelineViewportStateCreateInfo viewport_state_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = narrow_cast<std::uint32_t>(viewports.size()),
        .pViewports = viewports.data(),
        .scissorCount = narrow_cast<std::uint32_t>(scissors.size()),
        .pScissors = scissors.data(),
    };

    static ::VkPipelineRasterizationStateCreateInfo rasterization_state_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_FALSE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = VK_FALSE,
        .lineWidth = 1.f,
    };

    static ::VkPipelineMultisampleStateCreateInfo multisample_state_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = VK_FALSE,
        .alphaToCoverageEnable = VK_FALSE,
        .alphaToOneEnable = VK_FALSE,
    };

    static std::array<::VkPipelineColorBlendAttachmentState, 1>  //
        color_blend_attchment_state{
            ::VkPipelineColorBlendAttachmentState{
                .blendEnable = VK_FALSE,
            },
        };

    static ::VkPipelineColorBlendStateCreateInfo color_blend_state_info{
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .attachmentCount =
            narrow_cast<std::uint32_t>(color_blend_attchment_state.size()),
        .pAttachments = color_blend_attchment_state.data(),
        .blendConstants = {0.f, 0.f, 0.f, 0.f},
    };

    pipeline_ = vk::GraphicsPipeline{
        device,
        ::VkGraphicsPipelineCreateInfo{
            .stageCount = narrow_cast<std::uint32_t>(shader_stage_info.size()),
            .pStages = shader_stage_info.data(),
            .pVertexInputState = std::addressof(vertex_input_state_info),
            .pInputAssemblyState = std::addressof(input_assembly_state_info),
            .pTessellationState = nullptr,
            .pViewportState = std::addressof(viewport_state_info),
            .pRasterizationState = std::addressof(rasterization_state_info),
            .pMultisampleState = std::addressof(multisample_state_info),
            .pDepthStencilState = nullptr,
            .pColorBlendState = std::addressof(color_blend_state_info),
            .pDynamicState = nullptr,
            .layout = pipeline_layout,
            .renderPass = render_pass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE,
            .basePipelineIndex = -1,
        }};
  }

  vk::GraphicsPipeline pipeline_;
};

//------------------------------------------------------------------------------
class ShaderModule final {
 public:
  DECLARE_COPY_DELETE(ShaderModule);
  DECLARE_MOVE_DEFAULT(ShaderModule);

  ShaderModule() = delete;
  ~ShaderModule() = default;

  operator ::VkShaderModule() const { return shader_module_; }

 private:
  friend class Device;

  explicit ShaderModule(::VkDevice device,
                        const std::vector<std::uint32_t>& shader_spirv_bin) {
    shader_module_ = vk::ShaderModule{
        device, ::VkShaderModuleCreateInfo{
                    // Byte count.
                    .codeSize = shader_spirv_bin.size() * sizeof(std::uint32_t),
                    .pCode = shader_spirv_bin.data(),
                }};
  }

  vk::ShaderModule shader_module_;
};

//------------------------------------------------------------------------------
class Swapchain final {
 public:
  DECLARE_COPY_DELETE(Swapchain);
  DECLARE_MOVE_DEFAULT(Swapchain);

  Swapchain() = delete;
  ~Swapchain() = default;

  operator ::VkSwapchainKHR() const { return swapchain_; }

  std::vector<::VkImageView> create_image_views() {
    std::vector<::VkImageView> result;
    for (auto&& image_view : image_views_) {
      result.push_back(image_view);
    }
    return result;
  }

 private:
  friend class Device;

  explicit Swapchain(::VkDevice device,                                       //
                     std::vector<std::uint32_t> queue_families,               //
                     ::VkSurfaceKHR surface,                                  //
                     const ::VkSurfaceCapabilitiesKHR& surface_capabilities,  //
                     const ::VkSurfaceFormatKHR& surface_format,              //
                     ::VkPresentModeKHR surface_present_mode,                 //
                     ::VkSwapchainKHR previous_swapchain)
      : queue_families_{std::move(queue_families)},
        surface_{surface},
        surface_capabilities_{surface_capabilities},
        surface_format_{surface_format} {
    VkCompositeAlphaFlagBitsKHR composite_alpha =
        vk::find_first_flag(surface_capabilities_.supportedCompositeAlpha,
                            {VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
                             VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
                             VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
                             VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR},
                            static_cast<::VkCompositeAlphaFlagBitsKHR>(-1));
    CHECK_INVARIANT(composite_alpha != -1);

    swapchain_ = vk::Swapchain{
        device, ::VkSwapchainCreateInfoKHR{
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
                    .queueFamilyIndexCount =
                        narrow_cast<std::uint32_t>(queue_families_.size()),
                    .pQueueFamilyIndices = queue_families_.data(),
                    .preTransform = surface_capabilities_.currentTransform,
                    .compositeAlpha = composite_alpha,
                    .presentMode = surface_present_mode,
                    .clipped = VK_TRUE,
                    .oldSwapchain = previous_swapchain,
                }};

    swapchain_images_ = vk::SwapchainImages{device, swapchain_};

    for (auto&& swapchain_image : swapchain_images_()) {
      image_views_.push_back(
          ImageView{device, swapchain_image, surface_format.format});
    }
  }

  vk::Swapchain swapchain_;
  vk::SwapchainImages swapchain_images_;

  std::vector<ImageView> image_views_;
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

  void wait_for_idle() {
    ::VkResult result = ::vkDeviceWaitIdle(device_);
    CHECK_POSTCONDITION(result == VK_SUCCESS);
  }

  Queue create_queue() {
    CHECK_PRECONDITION(queue_families_.size() == 1);
    return Queue{device_, queue_families_.front(), 0u};
  }

  Buffer create_buffer(::VkDeviceSize requested_byte_count,
                       ::VkBufferUsageFlags requested_buffer_usage) {
    return Buffer{
        device_,
        requested_byte_count,
        requested_buffer_usage,
    };
  }

  DeviceMemory allocate_device_memory(
      const Buffer& buffer, ::VkMemoryPropertyFlags required_memory_flags) {
    bool found = false;

    std::uint32_t memory_type_index = 0;
    for (; memory_type_index < (sizeof(std::uint32_t) * 8) &&
           memory_type_index < phys_device_memory_properties_().memoryTypeCount;
         ++memory_type_index) {
      if ((buffer.memory_requirements_().memoryTypeBits &
           (1u << memory_type_index)) &&
          vk::has_all_flags(phys_device_memory_properties_()
                                .memoryTypes[memory_type_index]
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

  CommandBufferBlock allocate_command_buffer_block(::VkCommandPool command_pool,
                                                   std::uint32_t count) {
    return CommandBufferBlock{device_, command_pool, count};
  }

  CommandPool create_command_pool(std::uint32_t queue_family_index) {
    return CommandPool{device_, queue_family_index};
  }

  RenderPass create_render_pass(::VkFormat requested) {
    CHECK_PRECONDITION(std::any_of(surface_formats_().begin(),
                                   surface_formats_().end(),
                                   [requested](::VkSurfaceFormatKHR supported) {
                                     return supported.format == requested;
                                   }));
    return RenderPass{device_, requested};
  }

  template <typename DoRecreateSwapchainType, typename DoRenderType>
  std::unique_ptr<SurfaceRenderer> create_surface_renderer(  //
      DoRecreateSwapchainType&& recreate_swapchain,          //
      DoRenderType&& render) {
    return std::make_unique<SurfaceRenderer>(                       //
        surface_,                                                   //
        surface_capabilities_,                                      //
        surface_formats_,                                           //
        std::forward<DoRecreateSwapchainType>(recreate_swapchain),  //
        std::forward<DoRenderType>(render));
  }

  ShaderModule create_shader_module(
      const std::vector<std::uint32_t>& shader_spirv_bin) {
    return ShaderModule{device_, shader_spirv_bin};
  }

  Swapchain create_swapchain(                   //
      ::VkFormat requested_format,              //
      ::VkPresentModeKHR surface_present_mode,  //
      ::VkSwapchainKHR previous_swapchain = VK_NULL_HANDLE) {
    auto surface_format_iter =
        std::find_if(surface_formats_().begin(), surface_formats_().end(),
                     [requested_format](::VkSurfaceFormatKHR supported) {
                       return supported.format == requested_format;
                     });
    CHECK_PRECONDITION(surface_format_iter != surface_formats_().end());
    CHECK_PRECONDITION(std::find(surface_present_modes_().begin(),  //
                                 surface_present_modes_().end(),    //
                                 surface_present_mode) !=
                       surface_present_modes_().end());

    return Swapchain{device_,                //
                     queue_families_,        //
                     surface_,               //
                     surface_capabilities_,  //
                     *surface_format_iter,   //
                     surface_present_mode,   //
                     previous_swapchain};
  }

  std::vector<Framebuffer> create_framebuffers(
      ::VkRenderPass render_pass, std::span<::VkImageView> image_views) {
    std::vector<Framebuffer> result;
    for (auto&& image_view : image_views) {
      result.push_back(Framebuffer{device_, render_pass, image_view,
                                   surface_capabilities_().currentExtent});
    }
    return result;
  }

  PipelineLayout create_pipeline_layout() {
    return PipelineLayout{device_};  //
  }

  GraphicsPipeline create_graphics_pipeline(::VkShaderModule vertex_shader,
                                            ::VkShaderModule fragment_shader,
                                            ::VkPipelineLayout pipeline_layout,
                                            ::VkRenderPass render_pass) {
    return GraphicsPipeline{device_,          //
                            vertex_shader,    //
                            fragment_shader,  //
                            pipeline_layout,  //
                            render_pass,      //
                            surface_capabilities_().currentExtent};
  }

  std::vector<Semaphore> create_semaphores(std::uint32_t count) {
    std::vector<Semaphore> result;
    for (std::uint32_t i = 0; i < count; ++i) {
      result.push_back(Semaphore{device_});
    }
    return result;
  }

  std::vector<Fence> create_fences(std::uint32_t count,
                                   ::VkFenceCreateFlags flags = 0) {
    std::vector<Fence> result;
    for (std::uint32_t i = 0; i < count; ++i) {
      result.push_back(Fence{device_, flags});
    }
    return result;
  }

 private:
  friend class Instance;

  explicit Device(
      ::VkInstance instance,
      ::VkSurfaceKHR surface,                                       //
      ::VkPhysicalDevice phys_device,                               //
      const ::VkPhysicalDeviceFeatures& features,                   //
      const ::VkPhysicalDeviceMemoryProperties& memory_properties,  //
      std::vector<const char*> device_extensions,                   //
      std::vector<std::uint32_t> queue_families)
      : phys_device_features_{features},
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

    device_ = vk::Device{
        phys_device,
        ::VkDeviceCreateInfo{
            .queueCreateInfoCount =
                narrow_cast<std::uint32_t>(device_queue_infos_.size()),
            .pQueueCreateInfos = device_queue_infos_.data(),
            .enabledExtensionCount =
                narrow_cast<std::uint32_t>(device_extensions_.size()),
            .ppEnabledExtensionNames = device_extensions_.data(),
            .pEnabledFeatures = phys_device_features_.address(),
        }};

    surface_ = vk::Surface{instance, surface};
    surface_formats_ = vk::PhysicalDeviceSurfaceFormats{phys_device, surface};
    surface_present_modes_ =
        vk::PhysicalDeviceSurfacePresentModes{phys_device, surface};
    surface_capabilities_ =
        vk::PhysicalDeviceSurfaceCapabilities{phys_device, surface};

    std::print("Surface Formats: \n");
    for (auto&& surface_format : surface_formats_()) {
      std::print(" :: {}\n", vk::convert_to_string(surface_format.format));
    }

    std::print("Surface Present Modes: \n");
    for (auto&& surface_present_mode : surface_present_modes_()) {
      std::print(" '' {}\n", vk::convert_to_string(surface_present_mode));
    }

    std::print("Surface Capabilities: \n");
    std::print(" .. Image Count: {},{}\n",  //
               surface_capabilities_().minImageCount,
               surface_capabilities_().maxImageCount);
    std::print(" .. Image Extent Current: {},{}\n",  //
               surface_capabilities_().currentExtent.width,
               surface_capabilities_().currentExtent.height);
  }

  vk::Device device_;
  vk::Surface surface_;

  vk::PhysicalDeviceSurfaceFormats surface_formats_;
  vk::PhysicalDeviceSurfacePresentModes surface_present_modes_;
  vk::PhysicalDeviceSurfaceCapabilities surface_capabilities_;

  vk::PhysicalDeviceFeatures phys_device_features_;
  vk::PhysicalDeviceMemoryProperties phys_device_memory_properties_;

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
  ~Instance() = default;

  operator ::VkInstance() const { return instance_; }

  Device create_presentation_device(::VkSurfaceKHR surface) {
    CHECK_PRECONDITION(surface != VK_NULL_HANDLE);

    std::vector<FindQueueFamilyResult> selected_result = select_queue_family_if(
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

    return Device{instance_,
                  surface,
                  selected_phys_device,
                  phys_device_features_[selected_phys_device],
                  phys_device_memory_properties_[selected_phys_device],
                  {impl::SWAPCHAIN_EXTENSION_NAME},
                  {selected_queue_family_index}};
  }

 private:
  friend class Application;

  explicit Instance(const ::VkApplicationInfo* app_info,  //
                    std::vector<const char*> layers,      //
                    std::vector<const char*> extensions,  //
                    DebugLevel debug_level)
      : instance_layers_{std::move(layers)},
        instance_extensions_{std::move(extensions)} {
    std::print("Requested Layers: \n");
    for (auto&& layer : instance_layers_) {
      std::print(" == {}\n", layer);
    }

    std::print("Requested Extensions: \n");
    for (auto&& extension : instance_extensions_) {
      std::print(" -- {}\n", extension);
    }

    ::VkInstanceCreateInfo create_info{
        .pApplicationInfo = app_info,
        .enabledLayerCount =
            narrow_cast<std::uint32_t>(instance_layers_.size()),
        .ppEnabledLayerNames = instance_layers_.data(),
        .enabledExtensionCount =
            narrow_cast<std::uint32_t>(instance_extensions_.size()),
        .ppEnabledExtensionNames = instance_extensions_.data(),
    };

    const bool use_debug =
        debug_level != DebugLevel::NONE &&
        vk::has_string_name(instance_extensions_, impl::DEBUG_EXTENSION_NAME);

    if (use_debug) {
      create_info.pNext = debug_create_info_.address();
      debug_create_info_().messageSeverity =
          ConvertToDebugSeverity(debug_level);
      debug_create_info_().messageType =
          VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
      debug_create_info_().pfnUserCallback = debug_messenger_callback;
    }

    instance_ = vk::Instance{create_info};

    if (use_debug) {
      debug_ = vk::DebugMessenger(instance_, debug_create_info_());
    }

    phys_devices_ = vk::PhysicalDevices{instance_};

    std::print("Physical Devices: \n");
    for (auto&& phys_device : phys_devices_()) {
      phys_device_properties_[phys_device] =
          vk::PhysicalDeviceProperties{phys_device};
      phys_device_memory_properties_[phys_device] =
          vk::PhysicalDeviceMemoryProperties{phys_device};
      phys_device_features_[phys_device] =
          vk::PhysicalDeviceFeatures{phys_device};
      phys_device_queue_family_properties_[phys_device] =
          vk::PhysicalDeviceQueueFamilyProperties{phys_device};
      supported_device_extension_properties_[phys_device] =
          vk::DeviceExtensionProperties{phys_device, nullptr};

      std::print(" ** {} [{}]\n",
                 phys_device_properties_[phys_device]().deviceName,
                 vk::convert_to_string(
                     phys_device_properties_[phys_device]().deviceType));

      std::print("Queue Family Flags: \n");
      for (auto&& property :
           phys_device_queue_family_properties_[phys_device]()) {
        std::print(" .. [{}] {}\n", property.queueCount,
                   vk::convert_to_string(property.queueFlags));
      }

      std::print("Supported Device Extensions: \n");
      for (auto&& property :
           supported_device_extension_properties_[phys_device]()) {
        std::print(" -- {}\n", property.extensionName);
      }

      CHECK_INVARIANT(vk::has_extension_property(
          supported_device_extension_properties_[phys_device],
          impl::SWAPCHAIN_EXTENSION_NAME));
    }
  }

  struct FindQueueFamilyResult final {
    ::VkPhysicalDevice phys_device = VK_NULL_HANDLE;
    std::uint32_t queue_family_index = 0;
  };

  template <typename PredicateType>
  std::vector<FindQueueFamilyResult> select_queue_family_if(
      PredicateType&& predicate) {
    std::vector<FindQueueFamilyResult> result;

    for (auto&& phys_device : phys_devices_()) {
      auto&& phys_device_property = phys_device_properties_[phys_device];
      auto&& queue_family_properties =
          phys_device_queue_family_properties_[phys_device]();

      for (std::uint32_t queue_family_i = 0;                 //
           queue_family_i < queue_family_properties.size();  //
           ++queue_family_i) {
        auto&& queue_family_property = queue_family_properties[queue_family_i];

        if (predicate(phys_device,           //
                      phys_device_property,  //
                      queue_family_i,        //
                      queue_family_property)) {
          result.push_back({
              .phys_device = phys_device,
              .queue_family_index = queue_family_i,
          });
        }
      }
    }

    return result;
  }

  vk::Instance instance_;

  vk::DebugMessenger debug_;
  vk::DebugUtilsMessengerCreateInfo debug_create_info_;

  std::vector<const char*> instance_layers_;
  std::vector<const char*> instance_extensions_;

  vk::PhysicalDevices phys_devices_;

  std::map<::VkPhysicalDevice, vk::PhysicalDeviceProperties>
      phys_device_properties_;
  std::map<::VkPhysicalDevice, vk::PhysicalDeviceMemoryProperties>
      phys_device_memory_properties_;
  std::map<::VkPhysicalDevice, vk::PhysicalDeviceFeatures>
      phys_device_features_;
  std::map<::VkPhysicalDevice, vk::PhysicalDeviceQueueFamilyProperties>
      phys_device_queue_family_properties_;
  std::map<::VkPhysicalDevice, vk::DeviceExtensionProperties>
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

  static VKAPI_ATTR ::VkBool32 debug_messenger_callback(
      ::VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
      ::VkDebugUtilsMessageTypeFlagsEXT message_type,
      const ::VkDebugUtilsMessengerCallbackDataEXT* data, void*) {
    CHECK_PRECONDITION(
        data->sType ==
        VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CALLBACK_DATA_EXT);
    std::print("[{}] <{}> {}\n", vk::convert_to_string(message_severity),
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

  Instance create_instance(std::span<const char*> requested_layers = {},
                           std::span<const char*> requested_extensions = {},
                           DebugLevel debug_level = DebugLevel::NONE) {
    std::vector<const char*> layers{requested_layers.begin(),
                                    requested_layers.end()};
    std::vector<const char*> extensions{requested_extensions.begin(),
                                        requested_extensions.end()};

    if (vk::has_layer_property(supported_layers_,
                               impl::VALIDATION_LAYER_NAME)) {
      layers.push_back(impl::VALIDATION_LAYER_NAME);
    }

    if (debug_level != DebugLevel::NONE) {
      if (vk::has_extension_property(
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
