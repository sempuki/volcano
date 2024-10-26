load("@rules_cc//cc:defs.bzl", "cc_binary", "cc_library", "cc_test")

package(default_visibility = ["//visibility:public"])

#-------------------------------------------------------------------------------

cc_library(
    name = "resource",
    hdrs = ["resource.hpp"],
    deps = [
        "//lib:base",
    ],
)

cc_test(
    name = "resource_test",
    srcs = ["resource_test.cpp"],
    deps = [
        ":resource",
        "//lib:testing",
    ],
)

