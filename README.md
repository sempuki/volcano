# volcano

Vulkan rendering experiments on GLFW.

## Build

Requires [Bazelisk](https://github.com/bazelbuild/bazelisk) (installed as
`bazel`); the Bazel release is pinned in `.bazelversion`. GLFW and the Vulkan
headers come from the Bazel Central Registry and the shared core libraries from
the [lib](https://github.com/sempuki/lib) submodule. The Vulkan loader is not
in the registry, so install it from the platform: the distribution's
`vulkan-loader` package on Linux, or the LunarG Vulkan SDK on macOS and Windows.

```sh
git clone --recurse-submodules git@github.com:sempuki/volcano.git
# or, in an existing clone:
git submodule update --init

bazel test //...
# Tests that open a window need the session's display:
bazel test --config=display //engine:glfw_window_test //engine:integration_test
bazel run //:hello
```

Code targets C++26; flags come from `@lib//bazel:copts.bzl`.
