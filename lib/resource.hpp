#pragma once

#include "lib/base.hpp"

namespace volc::lib {

class Factory;

class Instance final {
 public:
  DECLARE_COPY_DELETE(Instance);
  DECLARE_MOVE_DEFAULT(Instance);

  Instance() = delete;
  ~Instance() = default;

 private:
  VkInstance instance_{};
};

class Factory final {
 public:
  DECLARE_COPY_DELETE(Factory);
  DECLARE_MOVE_DEFAULT(Factory);

  Factory() = default;
  ~Factory() = default;

 private:
};

}  // namespace volc::lib
