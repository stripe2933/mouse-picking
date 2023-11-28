# mouse-picking

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Pixel-perfect mouse picking with OpenGL stencil buffer.

## Build

```shell
git clone https://github.com/stripe2933/mouse-picking.git
cd mouse-picking
mkdir build
# You may don't need to specify CMAKE_TOOLCHAIN_FILE if you already have all dependencies.
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
./build/mouse-picking
```

# Dependencies

- fmt
- range-v3
- imgui
- imguizmo
- [OGLWrapper](https://github.com/stripe2933/OGLWrapper) - gl3w, glfw, glm, stb_image