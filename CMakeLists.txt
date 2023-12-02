cmake_minimum_required(VERSION 3.24)

project(mouse_picking)

add_executable(mouse_picking
    main.cpp
    AppWindow.cpp
)
target_compile_features(mouse_picking PRIVATE cxx_std_20)
target_include_directories(mouse_picking PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/extlibs)

find_package(fmt CONFIG REQUIRED)
find_package(range-v3 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(imguizmo CONFIG REQUIRED)

include(FetchContent)

message(STATUS "Fetching OGLWrapper...")
FetchContent_Declare(
    OGLWrapper
    GIT_REPOSITORY https://github.com/stripe2933/OGLWrapper.git
    GIT_TAG 1a5e1e1
)
FetchContent_MakeAvailable(OGLWrapper)

target_link_libraries(mouse_picking PRIVATE
    OGLWrapper
    imgui::imgui imguizmo::imguizmo
    fmt::fmt range-v3::range-v3
)

# Copy shader and asset files to executable folder.
add_custom_target(copy_assets COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_LIST_DIR}/copy_assets.cmake)
add_dependencies(mouse_picking copy_assets)