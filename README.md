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

## Editor setup

clangd needs a `compile_commands.json`, and the headers it names must stay put.
Bazel's execution root does not: every build relinks it to only the external
repositories that build needed. lib's `bazel/lsp_mirror.py` builds in an output
base of its own, copies the headers clangd reads into `.lsp/mirror/` (ignored
by git and Bazel), and writes `compile_commands.json` against that mirror, so
builds and compiler switches never disturb your editor:

```sh
python3 2nd_party/lib/bazel/lsp_mirror.py                  # build the mirror now
python3 2nd_party/lib/bazel/lsp_mirror.py --if-stale       # only if anything changed
python3 2nd_party/lib/bazel/lsp_mirror.py --watch 60       # check every minute
python3 2nd_party/lib/bazel/lsp_mirror.py --install-hooks  # after checkout, merge, rebase
```

`--if-stale` takes a fraction of a second when nothing changed, so it is cheap
to run often. Restart clangd (`:LspRestart` in Neovim) after the first build.
