load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

cc_binary(
    name = "hello",
    srcs = ["hello.cpp"],
    deps = [
        "//lib:glfw_window",
        "//lib:render",
        "//lib:surface_render",
        "//lib:resource",
        "//shaders:shaders",
    ],
)

