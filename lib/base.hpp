#pragma once

#include <vulkan/vulkan.h>

#include <cstddef>
#include <cstdint>
#include <format>
#include <memory>
#include <source_location>
#include <utility>

#define DECLARE_COPY_DEFAULT(class_name__)              \
  class_name__(const class_name__&) noexcept = default; \
  class_name__& operator=(const class_name__&) noexcept = default;

#define DECLARE_COPY_DELETE(class_name__)     \
  class_name__(const class_name__&) = delete; \
  class_name__& operator=(const class_name__&) = delete;

#define DECLARE_MOVE_DEFAULT(class_name__)         \
  class_name__(class_name__&&) noexcept = default; \
  class_name__& operator=(class_name__&&) noexcept = default;

#define DECLARE_MOVE_DELETE(class_name__) \
  class_name__(class_name__&&) = delete;  \
  class_name__& operator=(class_name__&&) = delete;

#define DECLARE_USED(expression__) ((void)(sizeof(expression__)))
#define DECLARE_UNUSED(expression__) ((void)(sizeof(expression__)))

#define CHECK_CONTRACT__(condition__, kind__)                        \
  if (!(condition__)) [[unlikely]] {                                 \
    const auto current_location__ = std::source_location::current(); \
    throw std::logic_error(std::format(                              \
        "[{}:{}] Failed {}: {}", current_location__.file_name(),     \
        current_location__.function_name(), kind__, #condition__));  \
  }

#define CHECK_PRECONDITION(precondition__) \
  CHECK_CONTRACT__(precondition__, "Precondition")
#define CHECK_POSTCONDITION(postcondition__) \
  CHECK_CONTRACT__(postcondition__, "Postcondition")
#define CHECK_INVARIANT(invariant__) CHECK_CONTRACT__(invariant__, "Invariant")
#define CHECK_UNREACHABLE() CHECK_CONTRACT__(false, "Unreachable")
