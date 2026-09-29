// Copyright 2024 -- CONTRIBUTORS. See LICENSE.

#pragma once

// Volcano's vocabulary from the shared lib; its macros arrive with the include.
#include "base/core.hpp"

namespace volcano {
using lib::CheckedPointer;
using lib::demangle;
using lib::Depend;
using lib::dump_object_bytes;
using lib::Empty;
using lib::InOut;
using lib::invoke_with_continuation;
using lib::narrow_cast;
using lib::Out;
using lib::Overloaded;
using lib::to_type_string;
using lib::Unused;
using lib::unused;
}  // namespace volcano
