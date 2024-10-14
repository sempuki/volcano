#include "lib/glfw_window.hpp"
#include "lib/render.hpp"
#include "lib/resource.hpp"
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

using namespace volc;

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

  Application application{"hello", 0};
  glfw::PlatformWindow platform_window{"hello-window",
                                       {.width = 800, .height = 600}};

  auto instance = application.CreateInstance(
      {}, platform_window.RequiredExtensions(), DebugLevel::VERBOSE);
  auto surface = platform_window.CreateSurface(instance.Handle());
  auto device = instance.CreateDevice(surface);
  auto queue = device.CreateQueue();
  auto render_pass = device.CreateRenderPass(VK_FORMAT_B8G8R8A8_UNORM);
  auto vert_shader = device.CreateShaderModule(vertex_shader_spirv_bin);
  auto frag_shader = device.CreateShaderModule(fragment_shader_spirv_bin);
  auto pipeline_layout = device.CreatePipelineLayout();
  auto buffer =
      device.CreateBuffer(vert2f_color3f_pack.size() * sizeof(float) * 5,
                          VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

  ::vkDestroySurfaceKHR(instance.Handle(), surface, nullptr);
}
