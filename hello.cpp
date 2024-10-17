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
      reinterpret_cast<const std::byte*>(vert2f_color3f_pack.data()),
      vert_buffer_byte_count};

  Application application{"hello", 0};
  std::unique_ptr<Window> window = std::make_unique<glfw::PlatformWindow>(
      "hello-window", Window::Geometry{.width = 800, .height = 600}  //
  );

  auto instance = application.CreateInstance({}, window->RequiredExtensions(),
                                             DebugLevel::VERBOSE);
  auto surface = window->CreateSurface(instance.Handle());
  auto device = instance.CreateDevice(surface);
  auto queue = device.CreateQueue();
  auto render_pass = device.CreateRenderPass(VK_FORMAT_B8G8R8A8_UNORM);
  auto vert_shader = device.CreateShaderModule(vertex_shader_spirv_bin);
  auto frag_shader = device.CreateShaderModule(fragment_shader_spirv_bin);
  auto pipeline_layout = device.CreatePipelineLayout();
  auto buffer = device.CreateBuffer(vert_buffer_byte_count,
                                    VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
  auto memory = device.AllocateDeviceMemory(
      buffer, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT |
                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                  VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
  memory.CopyInitialize(vert_buffer_bytes);
  auto command_pool = device.CreateCommandPool(queue.FamilyIndex());

  std::unique_ptr<Renderer> renderer =
      std::make_unique<SurfaceRenderer>(surface);

  window->SetRenderer(std::move(renderer));
  window->Show();

  ::vkDestroySurfaceKHR(instance.Handle(), surface, nullptr);
}
