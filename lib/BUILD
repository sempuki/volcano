load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

#-------------------------------------------------------------------------------

cc_library(
    name = "base",
    hdrs = ["base.hpp"],
)

#-------------------------------------------------------------------------------

cc_library(
    name = "surface_render",
    hdrs = ["surface_render.hpp"],
    deps = [
        ":base",
        ":render",
    ],
)

#-------------------------------------------------------------------------------

cc_library(
    name = "glfw_window",
    hdrs = ["glfw_window.hpp"],
    deps = [
        ":base",
        ":window",
        ":render",
    ],
    linkopts = [
        "-lglfw",
    ],
)

cc_test(
    name = "glfw_window_test",
    srcs = ["glfw_window_test.cpp"],
    deps = [
        ":glfw_window",
        ":testing",
    ],
)

#-------------------------------------------------------------------------------

cc_test(
    name = "integration_test",
    srcs = ["integration_test.cpp"],
    deps = [
        ":glfw_window",
        ":render",
        ":resource",
        ":testing",
    ],
)

#-------------------------------------------------------------------------------

cc_library(
    name = "render",
    hdrs = ["render.hpp"],
    deps = [
        ":base",
    ],
)

cc_test(
    name = "render_test",
    srcs = ["render_test.cpp"],
    deps = [
        ":render",
        ":testing",
    ],
)

#-------------------------------------------------------------------------------

cc_library(
    name = "resource",
    hdrs = ["resource.hpp"],
    deps = [
        ":base",
        ":surface_render",
        "//vk:resource",
    ],
)

cc_test(
    name = "resource_test",
    srcs = ["resource_test.cpp"],
    deps = [
        ":resource",
        ":testing",
    ],
)

#-------------------------------------------------------------------------------

cc_library(
  name = "testing",
  hdrs= ["testing.hpp"],
  srcs= ["testing.cpp"],
  deps = [
        "@catch2",
        "@catch2//:catch2_main",
  ],
)

#-------------------------------------------------------------------------------

cc_library(
    name = "window",
    hdrs = ["window.hpp"],
    deps = [
        ":base",
    ],
)

cc_test(
    name = "window_test",
    srcs = ["window_test.cpp"],
    deps = [
        ":window",
        ":testing",
    ],
)

