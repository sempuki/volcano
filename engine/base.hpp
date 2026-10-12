// Copyright 2024 -- CONTRIBUTORS. See LICENSE.

#pragma once

// Utilities from the shared lib, declared in volcano's namespace; lib's macros
// arrive with the include. The vocabulary types are in core/argument.hpp and
// core/status.hpp.
#include "base/core.hpp"

namespace volcano {
using lib::CheckedPointer;
using lib::demangle;
using lib::dump_object_bytes;
using lib::Empty;
using lib::invoke_with_continuation;
using lib::narrow_cast;
using lib::Overloaded;
using lib::to_type_string;
using lib::Unused;
using lib::unused;
}  // namespace volcano
