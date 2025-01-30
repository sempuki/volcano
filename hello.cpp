#include "lib/glfw_window.hpp"
#include "lib/render.hpp"
#include "lib/resource.hpp"
#include "lib/surface_render.hpp"
#include "shaders/shaders.hpp"

#include <cmath>
#include <cstdlib>
#include <vector>

using namespace volcano;

struct Vertex2D {
  float position[2];
};

struct ColorF {
  float color[3];
};

struct Vertex2D_ColorF_pack {
  Vertex2D position;
  ColorF color;
};

struct SwapchainRenderContext final {
  DECLARE_COPY_DELETE(SwapchainRenderContext);
  DECLARE_MOVE_DELETE(SwapchainRenderContext);

  SwapchainRenderContext() = delete;
  ~SwapchainRenderContext() { device->wait_for_idle(); }

  SwapchainRenderContext(::VkExtent2D geometry,
                         const SwapchainRenderContext& previous)
      : SwapchainRenderContext(geometry,                    //
                               previous.vertex_buffers[0],  //
                               previous.vertex_count,       //
                               previous.vert_shader,        //
                               previous.frag_shader,        //
                               previous.swapchain,          //
                               previous.device,             //
                               previous.command_pool) {}

  SwapchainRenderContext(::VkExtent2D geometry,                //
                         ::VkBuffer vertex_buffer,             //
                         std::uint32_t vertex_count,           //
                         ::VkShaderModule vert_shader,         //
                         ::VkShaderModule frag_shader,         //
                         ::VkSwapchainKHR previous_swapchain,  //
                         InOut<Device> device,                 //
                         InOut<CommandPool> command_pool)
      : device{device},
        command_pool{command_pool},
        geometry{geometry},
        vertex_buffers{vertex_buffer},
        vertex_count{vertex_count},
        vert_shader{vert_shader},
        frag_shader{frag_shader},
        swapchain{device->create_swapchain(  //
            VK_FORMAT_B8G8R8A8_UNORM,        //
            VK_PRESENT_MODE_FIFO_KHR,        //
            previous_swapchain)},            //
        swapchain_image_views{swapchain.create_image_views()},
        render_pass{device->create_render_pass(VK_FORMAT_B8G8R8A8_UNORM)},
        framebuffers{device->create_framebuffers(  //
            render_pass,                           //
            swapchain_image_views)},
        pipeline_layout{device->create_pipeline_layout()},
        graphics_pipeline{device->create_graphics_pipeline(  //
            vert_shader,                                     //
            frag_shader,                                     //
            pipeline_layout,                                 //
            render_pass)},
        command_buffer_block{device->allocate_command_buffer_block(  //
            *command_pool,                                           //
            swapchain_image_views.size())} {
    for (std::uint32_t i = 0; i < framebuffers.size(); ++i) {
      render_pass_commands.push_back(
          command_buffer_block.create_render_pass_command_buffer(
              i, render_pass, framebuffers[i], framebuffers[i].extent()));
      render_pass_commands.back().bind(graphics_pipeline);
      render_pass_commands.back().bind(0, vertex_buffers,
                                       vertex_buffer_offsets);
      render_pass_commands.back().draw(vertex_count);
    }
  }

  InOut<Device> device;
  InOut<CommandPool> command_pool;
  ::VkExtent2D geometry;

  std::array<::VkBuffer, 1> vertex_buffers;
  std::array<::VkDeviceSize, 1> vertex_buffer_offsets{0};
  std::uint32_t vertex_count{0};

  ::VkShaderModule vert_shader;
  ::VkShaderModule frag_shader;

  Swapchain swapchain;
  std::vector<::VkImageView> swapchain_image_views;
  std::uint32_t next_swapchain_image_index{0};

  RenderPass render_pass;
  std::vector<Framebuffer> framebuffers;

  PipelineLayout pipeline_layout;
  GraphicsPipeline graphics_pipeline;

  CommandBufferBlock command_buffer_block;
  std::vector<RenderPassCommandBuffer> render_pass_commands;

  std::vector<Fence> submission_fences;
  std::vector<Semaphore> render_complete;
};

int main() {
  std::cout << "Hello world " << std::endl;

  const float vertex_scale = 1.6f;
  const std::vector<std::array<float, 5>> vert2f_color3f_pack = {
      // Counter-clockwise wound.
      {
          vertex_scale * 0.5f,                     // X
          vertex_scale * std::sqrt(3.0f) * 0.25f,  // Y
          1.0f, 0.0f, 0.0f                         // RGB
      },
      {
          vertex_scale * 0.0f,                      // X
          vertex_scale * -std::sqrt(3.0f) * 0.25f,  // Y
          0.0f, 1.0f, 0.0f                          // RGB
      },
      {
          vertex_scale * -0.5f,                    // X
          vertex_scale * std::sqrt(3.0f) * 0.25f,  // Y
          0.0f, 0.0f, 1.0f                         // RGB
      }};
  const std::size_t vertex_buffer_vertex_count = vert2f_color3f_pack.size();
  const std::size_t vertex_buffer_byte_count =  //
      vert2f_color3f_pack.size() *              //
      vert2f_color3f_pack.front().size() *      //
      sizeof(vert2f_color3f_pack.front().front());
  const std::span<const std::byte> vertex_buffer_bytes = {
      reinterpret_cast<const std::byte*>(vert2f_color3f_pack.data()),  //
      vertex_buffer_byte_count                                         //
  };

  Window::Geometry initial_window_geometry{.width = 800, .height = 600};
  std::unique_ptr<Window> window = std::make_unique<glfw::PlatformWindow>(
      "hello-window", initial_window_geometry);

  Application application{"hello", 0};
  auto instance = application.create_instance({}, window->required_extensions(),
                                              DebugLevel::VERBOSE);
  auto surface = window->create_surface(instance);
  auto device = instance.create_presentation_device(surface);
  auto vert_shader = device.create_shader_module(vertex_shader_spirv_bin);
  auto frag_shader = device.create_shader_module(fragment_shader_spirv_bin);

  auto vertex_buffer = device.create_buffer(vertex_buffer_byte_count,
                                            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  auto memory = device.allocate_device_memory(
      vertex_buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                         VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                         VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  memory.copy_initialize(vertex_buffer_bytes);

  auto queue = device.create_queue();
  auto command_pool = device.create_command_pool(queue.family_index());

  auto swapchain_render_context = std::make_unique<SwapchainRenderContext>(
      ::VkExtent2D{
          .width = initial_window_geometry.width,
          .height = initial_window_geometry.height,
      },
      vertex_buffer,               //
      vertex_buffer_vertex_count,  //
      vert_shader,                 //
      frag_shader,                 //
      VK_NULL_HANDLE,              //
      InOut(device),               //
      InOut(command_pool));

  window->set_renderer(device.create_surface_renderer(
      [&swapchain_render_context]  //
      (::VkExtent2D geometry) -> bool {
        swapchain_render_context = std::make_unique<SwapchainRenderContext>(
            geometry, *swapchain_render_context);
        return true;
      },
      [&swapchain_render_context]() {
        // 1. Wait on a submission fence.
        // 2. `vkAcquireNextImageKHR` the corresp. semaphore.
        // 3. `vkQueueSubmit` command buffers to graphics queue.
        // 4. `vkQueuePresentKHR` swapchain image to present queue.
      }));

  window->show();
}

