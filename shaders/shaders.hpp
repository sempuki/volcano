#pragma once

#include <cstdint>
#include <vector>

namespace volcano {

inline std::vector<std::uint32_t> vertex_shader_spirv_bin = {
#include "shaders/hello_triangle.vert.spv.inl"
};
inline std::vector<std::uint32_t> fragment_shader_spirv_bin = {
#include "shaders/hello_triangle.frag.spv.inl"
};

}  // namespace volcano
