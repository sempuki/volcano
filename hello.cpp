#include "lib/glfw_window.hpp"
#include "lib/render.hpp"
#include "lib/resource.hpp"
#include "lib/surface_render.hpp"
#include "shaders/shaders.hpp"

#include <cmath>
#include <cstdlib>
#include <vector>

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

using namespace volcano;

int main() {
  std::cout << "Hello world " << std::endl;

  const float vert_scale = 1.6f;
  const std::vector<std::array<float, 5>> vert2f_color3f_pack = {
      // Counter-clockwise wound.
      {
          vert_scale * 0.5f,                     // X
          vert_scale * std::sqrt(3.0f) * 0.25f,  // Y
          1.0f, 0.0f, 0.0f                       // RGB
      },
      {
          vert_scale * 0.0f,                      // X
          vert_scale * -std::sqrt(3.0f) * 0.25f,  // Y
          0.0f, 1.0f, 0.0f                        // RGB
      },
      {
          vert_scale * -0.5f,                    // X
          vert_scale * std::sqrt(3.0f) * 0.25f,  // Y
          0.0f, 0.0f, 1.0f                       // RGB
      }};
  const std::size_t vert_buffer_byte_count =  //
      vert2f_color3f_pack.size() *            //
      vert2f_color3f_pack.front().size() *    //
      sizeof(vert2f_color3f_pack.front().front());
  const std::span<const std::byte> vert_buffer_bytes = {
      reinterpret_cast<const std::byte*>(vert2f_color3f_pack.data()),  //
      vert_buffer_byte_count                                           //
  };

  Application application{"hello", 0};
  std::unique_ptr<Window> window = std::make_unique<glfw::PlatformWindow>(
      "hello-window", Window::Geometry{.width = 800, .height = 600}  //
  );

  auto instance = application.create_instance(  //
      {}, window->required_extensions(), DebugLevel::VERBOSE);
  auto surface = window->create_surface(instance);
  auto device = instance.create_device(surface);

  auto queue = device.create_queue();
  auto buffer = device.create_buffer(vert_buffer_byte_count,
                                     VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  auto memory = device.allocate_device_memory(
      buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  memory.copy_initialize(vert_buffer_bytes);

  auto swapchain = device.create_swapchain(VK_FORMAT_B8G8R8A8_UNORM,
                                           VK_PRESENT_MODE_FIFO_KHR);
  auto swapchain_image_views = swapchain.create_image_views();
  auto render_pass = device.create_render_pass(VK_FORMAT_B8G8R8A8_UNORM);
  auto frambuffers =
      device.create_framebuffers(render_pass, swapchain_image_views);
  auto command_pool = device.create_command_pool(queue.family_index());
  auto command_buffer_block = device.allocate_command_buffer_block(
      command_pool, swapchain_image_views.size());

  auto vert_shader = device.create_shader_module(vertex_shader_spirv_bin);
  auto frag_shader = device.create_shader_module(fragment_shader_spirv_bin);
  auto pipeline_layout = device.create_pipeline_layout();
  auto graphics_pipeline = device.create_graphics_pipeline(
      vert_shader, frag_shader, pipeline_layout, render_pass);

  window->set_renderer(device.create_surface_renderer());
  window->show();
}
