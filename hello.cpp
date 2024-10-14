#include "lib/glfw_window.hpp"
#include "lib/render.hpp"
#include "lib/resource.hpp"
#include "shaders/shaders.hpp"

#include <cstdlib>

using namespace volc;

int main() {
  std::cout << "Hello world " << std::endl;

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

  ::vkDestroySurfaceKHR(instance.Handle(), surface, nullptr);
}
