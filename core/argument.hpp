// Copyright 2026 -- CONTRIBUTORS. See LICENSE.

#pragma once

#include "base/core.hpp"

// lib's argument wrappers, declared in volcano's namespace so that volcano
// names them plainly. Out marks an argument a function writes, InOut one it
// reads and writes, and Depend one it keeps a reference to.
namespace volcano {

using lib::Depend;
using lib::InOut;
using lib::Out;

}  // namespace volcano
