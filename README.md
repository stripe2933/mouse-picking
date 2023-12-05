# mouse-picking

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Pixel-perfect mouse picking with OpenGL stencil buffer.
If you want another mouse picking example with custom FBO + instancing, see [instancing branch](https://github.com/stripe2933/mouse-picking/tree/instancing). It unlimits the available object count in this branch (255 at max).

https://github.com/stripe2933/mouse-picking/assets/63503910/fa524657-2389-49c6-9b50-822088d3ff83

## Build

```shell
git clone https://github.com/stripe2933/mouse-picking.git
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
