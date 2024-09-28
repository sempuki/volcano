#pragma once

#include <string>
#include <string_view>

#include "lib/base.hpp"

namespace volc::lib {
namespace impl {
inline ::VkApplicationInfo MakeApplicationInfo() {
  return ::VkApplicationInfo{.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO};
}
inline ::VkInstanceCreateInfo MakeInstanceInfo() {
  return ::VkInstanceCreateInfo{.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};
}
}  // namespace impl

class Application;

class Instance final {
 public:
  DECLARE_COPY_DELETE(Instance);
  DECLARE_MOVE_DEFAULT(Instance);

  Instance() = delete;
  ~Instance() = default;

 private:
  friend class Application;

  explicit Instance(const VkApplicationInfo* app_info) {
    auto _ = impl::MakeInstanceInfo();
    _.pApplicationInfo = app_info;
  }

  ::VkInstance instance_{};
};

class Application final {
 public:
  DECLARE_COPY_DELETE(Application);
  DECLARE_MOVE_DEFAULT(Application);

  explicit Application(std::string_view name) : info_{impl::MakeApplicationInfo()}, name_{name} {
    info_.pApplicationName = name_.c_str();
  }

  Application() = delete;
  ~Application() = default;

  Instance CreateInstance() { return Instance{std::addressof(info_)}; }

 private:
  ::VkApplicationInfo info_;
  std::string name_;
};

}  // namespace volc::lib
