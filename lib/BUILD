load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

#-------------------------------------------------------------------------------

cc_library(
    name = "base",
    hdrs = ["base.hpp"],
    linkopts = [
        "-lvulkan",
    ],
)

#-------------------------------------------------------------------------------

cc_library(
    name = "resource",
    hdrs = ["resource.hpp"],
    deps = [
        ":base",
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

