#include "vk/resource.hpp"

#include "lib/testing.hpp"

namespace volcano::vk {

//------------------------------------------------------------------------------

TEST_CASE("ApplicationInfo") {
  ApplicationInfo info;

  SECTION("ShoulHaveTypeValue") {
    REQUIRE(info().sType == VK_STRUCTURE_TYPE_APPLICATION_INFO);
  }

  SECTION("ShoulHaveAddressToVkStructure") {
    // Under Test.
    ::VkApplicationInfo* address = info.address();

    // Postcondition.
    REQUIRE(address != nullptr);
  }

  SECTION("ShoulHaveNoAddressAfterMove") {
    // Under Test.
    ApplicationInfo moved = std::move(info);

    // Postcondition.
    REQUIRE(info.address() == nullptr);
  }

  SECTION("ShoulHaveStableAddressAfterMove") {
    // Precondition.
    ::VkApplicationInfo* prev_address = info.address();
    ApplicationInfo moved = std::move(info);

    // Under Test.
    ::VkApplicationInfo* curr_address = moved.address();

    // Postcondition.
    REQUIRE(prev_address == curr_address);
  }
}

TEST_CASE("InstanceCreateInfo") {
  InstanceCreateInfo info;

  SECTION("ShoulHaveTypeValue") {
    REQUIRE(info().sType == VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO);
  }

  SECTION("ShoulHaveAddressToVkStructure") {
    // Under Test.
    ::VkInstanceCreateInfo* address = info.address();

    // Postcondition.
    REQUIRE(address != nullptr);
  }
}

TEST_CASE("DeviceCreateInfo") {
  DeviceCreateInfo info;

  SECTION("ShoulHaveTypeValue") {
    REQUIRE(info().sType == VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO);
  }

  SECTION("ShoulHaveAddressToVkStructure") {
    // Under Test.
    ::VkDeviceCreateInfo* address = info.address();

    // Postcondition.
    REQUIRE(address != nullptr);
  }
}

TEST_CASE("DeviceQueueCreateInfo") {
  DeviceQueueCreateInfo info;

  SECTION("ShoulHaveTypeValue") {
    REQUIRE(info().sType == VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO);
  }

  SECTION("ShoulHaveAddressToVkStructure") {
    // Under Test.
    ::VkDeviceQueueCreateInfo* address = info.address();

    // Postcondition.
    REQUIRE(address != nullptr);
  }
}

TEST_CASE("BufferCreateInfo") {
  BufferCreateInfo info;

  SECTION("ShoulHaveTypeValue") {
    REQUIRE(info().sType == VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO);
  }

  SECTION("ShoulHaveAddressToVkStructure") {
    // Under Test.
    ::VkBufferCreateInfo* address = info.address();

    // Postcondition.
    REQUIRE(address != nullptr);
  }
}

TEST_CASE("MemoryAllocateInfo") {
  MemoryAllocateInfo info;

  SECTION("ShoulHaveTypeValue") {
    REQUIRE(info().sType == VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO);
  }

  SECTION("ShoulHaveAddressToVkStructure") {
    // Under Test.
    ::VkMemoryAllocateInfo* address = info.address();

    // Postcondition.
    REQUIRE(address != nullptr);
  }
}

TEST_CASE("CommandPoolCreateInfo") {
  CommandPoolCreateInfo info;

  SECTION("ShoulHaveTypeValue") {
    REQUIRE(info().sType == VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO);
  }

  SECTION("ShoulHaveAddressToVkStructure") {
    // Under Test.
    ::VkCommandPoolCreateInfo* address = info.address();

    // Postcondition.
    REQUIRE(address != nullptr);
  }
}

TEST_CASE("ImageViewCreateInfo") {
  ImageViewCreateInfo info;

  SECTION("ShoulHaveTypeValue") {
    REQUIRE(info().sType == VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO);
  }

  SECTION("ShoulHaveAddressToVkStructure") {
    // Under Test.
    ::VkImageViewCreateInfo* address = info.address();

    // Postcondition.
    REQUIRE(address != nullptr);
  }
}

TEST_CASE("RenderPassCreateInfo") {
  RenderPassCreateInfo info;

  SECTION("ShoulHaveTypeValue") {
    REQUIRE(info().sType == VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO);
  }

  SECTION("ShoulHaveAddressToVkStructure") {
    // Under Test.
    ::VkRenderPassCreateInfo* address = info.address();

    // Postcondition.
    REQUIRE(address != nullptr);
  }
}

TEST_CASE("PipelineLayoutCreateInfo") {
  PipelineLayoutCreateInfo info;

  SECTION("ShoulHaveTypeValue") {
    REQUIRE(info().sType == VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO);
  }

  SECTION("ShoulHaveAddressToVkStructure") {
    // Under Test.
    ::VkPipelineLayoutCreateInfo* address = info.address();

    // Postcondition.
    REQUIRE(address != nullptr);
  }
}

TEST_CASE("ShaderModuleCreateInfo") {
  ShaderModuleCreateInfo info;

  SECTION("ShoulHaveTypeValue") {
    REQUIRE(info().sType == VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO);
  }

  SECTION("ShoulHaveAddressToVkStructure") {
    // Under Test.
    ::VkShaderModuleCreateInfo* address = info.address();

    // Postcondition.
    REQUIRE(address != nullptr);
  }
}

TEST_CASE("SwapchainCreateInfo") {
  SwapchainCreateInfo info;

  SECTION("ShoulHaveTypeValue") {
    REQUIRE(info().sType == VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR);
  }

  SECTION("ShoulHaveAddressToVkStructure") {
    // Under Test.
    ::VkSwapchainCreateInfoKHR* address = info.address();

    // Postcondition.
    REQUIRE(address != nullptr);
  }
}

//------------------------------------------------------------------------------

const std::string VALIDATION_LAYER{"VK_LAYER_KHRONOS_validation"};

TEST_CASE("InstanceLayerProperties") {
  InstanceLayerProperties enumerated;

  SECTION("ShouldEnumerateSome") {
    REQUIRE(enumerated().size());  //
  }

  SECTION("ShouldHaveLayerNames") {
    for (auto&& item : enumerated()) {
      REQUIRE(item.layerName != nullptr);
    }
  }

  SECTION("ShouldHaveValidationLayer") {
    // Under Test.
    auto iter = std::find_if(  //
        enumerated().begin(),  //
        enumerated().end(),    //
        [](auto&& _) { return _.layerName == VALIDATION_LAYER; });

    // Postcondition.
    REQUIRE(iter != enumerated().end());
  }
}

TEST_CASE("ExtensionProperties") {
  InstanceExtensionProperties enumerated{VALIDATION_LAYER.c_str()};

  SECTION("ShouldEnumerateSome") {
    REQUIRE(enumerated().size());  //
  }

  SECTION("ShouldHaveExtensionNames") {
    for (auto&& item : enumerated()) {
      REQUIRE(item.extensionName != nullptr);
    }
  }
}

//------------------------------------------------------------------------------

TEST_CASE("Instance Handle") {
  // Precondition.
  ApplicationInfo app_info{::VkApplicationInfo{
      .pApplicationName = "test",
      .apiVersion = VK_API_VERSION_1_3,
  }};

  // Under Test.
  Instance handle{::VkInstanceCreateInfo{
      .pApplicationInfo = app_info.address(),
  }};

  SECTION("ShoulHaveValidHandle") {
    // Under Test.
    REQUIRE(handle.handle() != VK_NULL_HANDLE);
  }

  SECTION("ShoulHaveValidCreateInfo") {
    // Under Test.
    REQUIRE(handle.create_info().pApplicationInfo == app_info.address());
  }
}

//------------------------------------------------------------------------------

TEST_CASE("PhysicalDevices") {
  // Precondition.
  ApplicationInfo app_info{::VkApplicationInfo{
      .pApplicationName = "test",
      .apiVersion = VK_API_VERSION_1_3,
  }};
  Instance instance{::VkInstanceCreateInfo{
      .pApplicationInfo = app_info.address(),
  }};

  // Under Test.
  PhysicalDevices enumerated{instance.handle()};

  SECTION("ShouldEnumerateSome") {
    REQUIRE(enumerated().size());  //
  }

  SECTION("ShouldBeValid") {
    for (auto&& item : enumerated()) {
      REQUIRE(item != VK_NULL_HANDLE);
    }
  }
}

TEST_CASE("DeviceExtensionProperties") {
  // Precondition.
  ApplicationInfo app_info{::VkApplicationInfo{
      .pApplicationName = "test",
      .apiVersion = VK_API_VERSION_1_3,
  }};
  Instance instance{::VkInstanceCreateInfo{
      .pApplicationInfo = app_info.address(),
  }};
  PhysicalDevices phys_devices{instance.handle()};

  // Under Test.
  DeviceExtensionProperties enumerated{
      phys_devices().front(),
      VALIDATION_LAYER.c_str(),
  };

  SECTION("ShouldEnumerateSome") {
    REQUIRE(enumerated().size());  //
  }

  SECTION("ShouldHaveExtensionNames") {
    for (auto&& item : enumerated()) {
      REQUIRE(item.extensionName != nullptr);
    }
  }
}

TEST_CASE("PhysicalDeviceQueueFamilyProperties") {
  // Precondition.
  ApplicationInfo app_info{::VkApplicationInfo{
      .pApplicationName = "test",
      .apiVersion = VK_API_VERSION_1_3,
  }};
  Instance instance{::VkInstanceCreateInfo{
      .pApplicationInfo = app_info.address(),
  }};
  PhysicalDevices phys_devices{instance.handle()};

  // Under Test.
  PhysicalDeviceQueueFamilyProperties enumerated{phys_devices().front()};

  SECTION("ShouldEnumerateSome") {
    REQUIRE(enumerated().size());  //
  }

  SECTION("ShouldHaveQueueFlags") {
    for (auto&& item : enumerated()) {
      REQUIRE(item.queueFlags != 0u);
    }
  }
}

//------------------------------------------------------------------------------

TEST_CASE("Device Handle") {
  // Precondition.
  ApplicationInfo app_info{::VkApplicationInfo{
      .pApplicationName = "test",
      .apiVersion = VK_API_VERSION_1_3,
  }};
  Instance instance{::VkInstanceCreateInfo{
      .pApplicationInfo = app_info.address(),
  }};
  PhysicalDevices phys_devices{instance.handle()};

  // Under Test.
  Device handle{phys_devices().front(), ::VkDeviceCreateInfo{}};

  SECTION("ShoulHaveValidHandle") {
    // Under Test.
    REQUIRE(handle.handle() != VK_NULL_HANDLE);
  }
}

}  // namespace volcano::vk
