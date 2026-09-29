#include "vk/resource.hpp"

#include <type_traits>
#include <vector>

#include "base/testing.hpp"
#include "catch2/catch_template_test_macros.hpp"

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
      REQUIRE(item.layerName[0] != '\0');
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
      REQUIRE(item.extensionName[0] != '\0');
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
      REQUIRE(item.extensionName[0] != '\0');
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

//------------------------------------------------------------------------------
// Fake handles, so that handle ownership can be tested without a driver.

namespace {

std::vector<int*> closed_handles;

struct FakeInfoHolder {
  int value = 0;
  const int& operator()() const { return value; }
};

::VkResult fake_open(const int& /*info*/, int*& /*handle*/) {
  return VK_SUCCESS;
}
void fake_close(int* const& handle) { closed_handles.push_back(handle); }

::VkResult fake_open_parented(int* /*parent*/, const int& /*info*/,
                              int*& /*handle*/) {
  return VK_SUCCESS;
}
void fake_close_parented(int* const& /*parent*/, int* const& handle) {
  closed_handles.push_back(handle);
}

using FakeHandle =
    impl::HandleBase<int*, int, FakeInfoHolder, fake_open, fake_close>;
using FakeParentedHandle =
    impl::ParentedHandleBase<int*, int*, int, FakeInfoHolder,
                             fake_open_parented, fake_close_parented>;

}  // namespace

TEMPLATE_TEST_CASE("HandleOwnership", "", FakeHandle, FakeParentedHandle) {
  closed_handles.clear();
  int parent = 0, a = 0, b = 0;

  auto make = [&parent](int* handle) {
    if constexpr (std::is_same_v<TestType, FakeHandle>) {
      return TestType{handle};
    } else {
      return TestType{&parent, handle};
    }
  };

  SECTION("ShouldCloseOnceGivenDestruction") {
    { auto handle = make(&a); }
    REQUIRE(closed_handles == std::vector<int*>{&a});
  }

  SECTION("ShouldCloseOnlyTargetGivenMoveConstruction") {
    {
      auto source = make(&a);
      auto target = std::move(source);
      REQUIRE(closed_handles.empty());
    }
    REQUIRE(closed_handles == std::vector<int*>{&a});
  }

  SECTION("ShouldCloseOldHandleGivenMoveAssignmentOverLiveHandle") {
    {
      auto target = make(&a);
      target = make(&b);
      REQUIRE(closed_handles == std::vector<int*>{&a});
    }
    REQUIRE(closed_handles == std::vector<int*>{&a, &b});
  }
}

//------------------------------------------------------------------------------

TEST_CASE("MaybeEnumerateProperties") {
  std::vector<int> properties;

  SECTION("ShouldKeepOnlyWrittenEntriesGivenCountShrinksBetweenCalls") {
    int calls = 0;
    auto enumerate = [&calls](std::uint32_t* count, int* data) -> ::VkResult {
      calls++;
      if (!data) {
        *count = 3;
        return VK_SUCCESS;
      }
      *count = 2;
      data[0] = 10;
      data[1] = 20;
      return VK_SUCCESS;
    };

    impl::maybe_enumerate_properties(enumerate, InOut(properties));

    REQUIRE(properties == std::vector<int>{10, 20});
  }

  SECTION("ShouldRetryGivenIncompleteResult") {
    int round = 0;
    auto enumerate = [&round](std::uint32_t* count, int* data) -> ::VkResult {
      if (!data) {
        *count = round == 0 ? 1 : 2;
        return VK_SUCCESS;
      }
      if (round++ == 0) {
        data[0] = 1;
        return VK_INCOMPLETE;
      }
      *count = 2;
      data[0] = 1;
      data[1] = 2;
      return VK_SUCCESS;
    };

    impl::maybe_enumerate_properties(enumerate, InOut(properties));

    REQUIRE(properties == std::vector<int>{1, 2});
  }

  SECTION("ShouldAppendAllEntriesGivenVoidEnumerator") {
    auto enumerate = [](std::uint32_t* count, int* data) {
      if (data) {
        data[0] = 7;
      }
      *count = 1;
    };

    impl::maybe_enumerate_properties(enumerate, InOut(properties));

    REQUIRE(properties == std::vector<int>{7});
  }
}

//------------------------------------------------------------------------------

TEST_CASE("ConvertToString") {
  SECTION("ShouldReturnUnknownGivenUnlistedPresentMode") {
    REQUIRE(convert_to_string(static_cast<::VkPresentModeKHR>(0x7FFF0000)) ==
            "UNKNOWN");
  }

  SECTION("ShouldReturnUnknownGivenUnlistedFormat") {
    REQUIRE(convert_to_string(static_cast<::VkFormat>(0x7FFF0000)) ==
            "UNKNOWN");
  }

  SECTION("ShouldReturnNameGivenListedPresentMode") {
    REQUIRE(convert_to_string(VK_PRESENT_MODE_FIFO_KHR) ==
            "VK_PRESENT_MODE_FIFO_KHR");
  }
}

}  // namespace volcano::vk
