# mouse-picking

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Pixel-perfect mouse picking with render attachment. Support instanced rendering and framebuffer recreation when resized.
If you want mouse picking with stencil buffer (needs multiple draw calls), see [the main branch](https://github.com/stripe2933/mouse-picking).

https://github.com/stripe2933/mouse-picking/assets/63503910/a8abaf97-7494-4a6f-bf0b-be66443003cb

## Build

```shell
git clone https://github.com/stripe2933/mouse-picking.git
git checkout instancing
cd mouse-picking
mkdir build
# You may don't need to specify CMAKE_TOOLCHAIN_FILE if you already have all dependencies.
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
./build/mouse_picking
```

# Dependencies

- fmt
- range-v3
- imgui
- imguizmo
- [OGLWrapper](https://github.com/stripe2933/OGLWrapper) - gl3w, glfw, glm, stb_image, eventpp
