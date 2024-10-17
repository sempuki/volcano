#pragma once

#include <vulkan/vulkan.h>

static_assert(VK_HEADER_VERSION >= 290, "Update vulkan header version.");

#include <cstddef>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <limits>
#include <memory>
#include <print>
#include <source_location>
#include <string>
#include <string_view>
#include <type_traits>
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
        "[{}] {} Failed {}: {}", current_location__.file_name(),     \
        current_location__.function_name(), kind__, #condition__));  \
  }

#define CHECK_PRECONDITION(precondition__) \
  CHECK_CONTRACT__(precondition__, "Precondition")
#define CHECK_POSTCONDITION(postcondition__) \
  CHECK_CONTRACT__(postcondition__, "Postcondition")
#define CHECK_INVARIANT(invariant__) CHECK_CONTRACT__(invariant__, "Invariant")
#define CHECK_UNREACHABLE() CHECK_CONTRACT__(false, "Unreachable")

namespace volcano {

template <typename Type>
class CheckedPointer {
 public:
  DECLARE_COPY_DEFAULT(CheckedPointer);
  DECLARE_MOVE_DEFAULT(CheckedPointer);

  CheckedPointer() = default;
  ~CheckedPointer() = default;

  explicit CheckedPointer(Type& value) : arg_{&value} {}
  template <typename Derived>
    requires(std::is_base_of_v<Type, Derived> && !std::is_same_v<Type, Derived>)
  CheckedPointer(CheckedPointer<Derived> that) : arg_{that.get()} {}

  explicit operator bool() const { return arg_; }

  Type& operator*() const {
    CHECK_PRECONDITION(arg_);
    return *arg_;
  }
  Type* operator->() const {
    CHECK_PRECONDITION(arg_);
    return arg_;
  }
  Type* get() const {
    CHECK_PRECONDITION(arg_);
    return arg_;
  }

 protected:
  Type* arg_ = nullptr;
};

struct Unused {};
inline constexpr Unused unused;

// Out argument:
// - May be explicitly initialized.
// - May be implicitly unused.
// - May upcast.
// - May be forwarded by copy/move.
// - May *not* manage lifetimes.
// - May *not* be converted to InOut.
template <typename Type>
class Out : public CheckedPointer<Type> {
  using BaseType = CheckedPointer<Type>;

 public:
  DECLARE_COPY_DEFAULT(Out);
  DECLARE_MOVE_DEFAULT(Out);

  Out() = delete;
  ~Out() = default;

  Out(Unused) {}
  explicit Out(Type& value) : BaseType{value} {}

  template <typename Derived>
    requires(std::is_base_of_v<Type, Derived> && !std::is_same_v<Type, Derived>)
  Out(Out<Derived> that) : BaseType{that} {}
};

// In-out argument:
// - Must be explicitly initialized.
// - May *not* be unused.
// - May upcast.
// - May be forwarded by copy/move.
// - May *not* manage lifetimes.
// - May be converted to Out.
template <typename Type>
class InOut : public Out<Type> {
  using BaseType = Out<Type>;

 public:
  DECLARE_COPY_DEFAULT(InOut);
  DECLARE_MOVE_DEFAULT(InOut);

  InOut() = delete;
  ~InOut() = default;

  explicit InOut(Type& value) : BaseType{value} { CHECK_INVARIANT(this->arg_); }

  template <typename Derived>
    requires(std::is_base_of_v<Type, Derived> && !std::is_same_v<Type, Derived>)
  InOut(InOut<Derived> that) : BaseType{that} {
    CHECK_INVARIANT(this->arg_);
  }
};

// Class member dependency:
// - Must be explicitly initialized.
// - May *not* be unused.
// - May upcast.
// - May be forwarded by copy/move.
// - May *not* manage lifetimes.
// - May be converted to InOut.
template <typename Type>
class Depend final : public InOut<Type> {
  using BaseType = InOut<Type>;

 public:
  using BaseType::BaseType;
};

template <typename T>
Out(T& value) -> Out<T>;
template <typename T>
InOut(T& value) -> InOut<T>;
template <typename T>
Depend(T& value) -> Depend<T>;

template <class... CallableTypes>
struct Overloaded : CallableTypes... {
  using CallableTypes::operator()...;
};

template <class... Ts>
Overloaded(Ts...) -> Overloaded<Ts...>;

template <typename CallableType, typename... ArgumentTypes,
          typename ContinuationType>
void InvokeWithContinuation(ContinuationType&& continuation,
                            CallableType&& callable,
                            ArgumentTypes&&... arguments) {
  using CallableResultType =
      std::invoke_result_t<CallableType, ArgumentTypes...>;

  if constexpr (std::is_invocable_v<ContinuationType, CallableResultType>) {
    std::invoke(
        continuation,
        std::invoke(callable, std::forward<ArgumentTypes>(arguments)...));
  } else {
    std::invoke(callable, std::forward<ArgumentTypes>(arguments)...);
    std::invoke(continuation);
  }
}

}  // namespace volcano
