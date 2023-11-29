//
// Created by gomkyung2 on 11/26/23.
//

#pragma once

#include <vector>

#include <OGLWrapper/Shader.hpp>
#include <OGLWrapper/Program.hpp>
#include <OGLWrapper/OpenGLContext.hpp>
#include <OGLWrapper/Helper/Image.hpp>
#include <OGLWrapper/GLFW/GlfwWindow.hpp>

#include <glm/ext/matrix_float4x4.hpp>

#include <DirtyProperty.hpp>

#include "VertexPNT.hpp"
#include "Mesh.hpp"
#include "Material.hpp"

class AppWindow final : public GlfwWindow {
    // This struct should be at the top of the class declaration, because it is intended to be initialized before
    // the constructor.
    OGLWrapper::OpenGLContext context {};

    const OGLWrapper::Shader<OGLWrapper::ShaderType::Vertex> vertex_shader { std::filesystem::path { "shaders/pnt.vert"} };
    const OGLWrapper::Program primary_program {
        vertex_shader,
        OGLWrapper::Shader<OGLWrapper::ShaderType::Fragment> {std::filesystem::path { "shaders/blinn_phong.frag"} }
    };
    const OGLWrapper::Program outliner_program {
        vertex_shader,
        OGLWrapper::Shader<OGLWrapper::ShaderType::Fragment> {std::filesystem::path { "shaders/solid.frag"} }
    };

    const Material wood {
        Image { "assets/textures/container2.png" }.toTexture(),
        Image { "assets/textures/container2_specular.png" }.toTexture(),
    };

    GpuFacetedMesh<VertexPNT> cube_mesh;

    static constexpr std::uint8_t no_hover_index = 0xFF;
    std::uint8_t hovered_index = no_hover_index;

    std::vector<glm::vec3> rotation_axes;
    std::vector<glm::mat4> models;

    // View/projection related properties.
    static constexpr float camera_distance = 10.f;
    static constexpr glm::vec3 world_up { 0.f, 1.f, 0.f };
    DirtyProperty<glm::mat4> view;

    DirtyProperty<float> aspect { 1.f };
    DirtyProperty<float> fov { 45.f };
    DirtyProperty<glm::mat4> projection;

    // Window event handlers.
    void onFramebufferSizeCallback(glm::ivec2 size) override;
    void onScrollCallback(glm::dvec2 offset) override;
    void onCursorPosCallback(glm::dvec2 position) override;
    void onKeyCallback(int key, int scancode, int action, int mods) override;

    // Render loop related functions.
    std::optional<glm::vec3> camera_velocity;

    void update(float time_delta);
    void updateImGui(float time_delta);
    void draw() const;
    void drawImGui() const;
    void onRenderLoop(float time_delta) override;

    void initImGui();
    void initModels();

    static GpuFacetedMesh<VertexPNT> loadCubeMesh();

public:
    AppWindow();
    ~AppWindow() override;
};
