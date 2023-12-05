//
// Created by gomkyung2 on 11/26/23.
//

#pragma once

#include <vector>

#include <OGLWrapper/Shader.hpp>
#include <OGLWrapper/Program.hpp>
#include <OGLWrapper/OpenGLContext.hpp>
#include <OGLWrapper/Framebuffer.hpp>
#include <OGLWrapper/Helper/Image.hpp>
#include <OGLWrapper/Helper/Mesh.hpp>
#include <OGLWrapper/GLFW/Window.hpp>

#include <glm/ext/matrix_float4x4.hpp>

#include <DirtyProperty.hpp>

#include "Vertex.hpp"
#include "Material.hpp"
#include "Mesh.hpp"

struct VpMatrix {
    glm::mat4 projection_view;
    alignas(glm::vec4) glm::vec3 view_pos;
};

class AppWindow final : public OGLWrapper::GLFW::Window {
    // This struct should be at the top of the class declaration, because it is intended to be initialized before
    // the constructor.
    const OGLWrapper::OpenGLContext context {};

    const OGLWrapper::Program primary_instanced_program {
        OGLWrapper::VertexShader { "shaders/cube_instanced.vert" },
        OGLWrapper::FragmentShader { "shaders/cube_instanced.frag" }
    };
    const OGLWrapper::Program outliner_instanced_program {
        OGLWrapper::VertexShader { "shaders/outliner_instanced.vert" },
        OGLWrapper::FragmentShader { "shaders/outliner_instanced.frag" }
    };

    OGLWrapper::Framebuffer<GL_FRAMEBUFFER> framebuffer;
    OGLWrapper::Texture<GL_TEXTURE_2D> color_attachment;
    OGLWrapper::Renderbuffer depth_attachment;
    OGLWrapper::Texture<GL_TEXTURE_2D> instance_id_attachment;

    const Material wood {
        OGLWrapper::Helper::Image { "assets/textures/container2.png" }.toTexture<GL_TEXTURE_2D>(),
        OGLWrapper::Helper::Image { "assets/textures/container2_specular.png" }.toTexture<GL_TEXTURE_2D>(),
    };

    OGLWrapper::Helper::GpuMesh<VertexPNT> cube_mesh { loadCubeMesh() };
    GpuInstancedMesh<VertexPNT, glm::mat4> cube_instanced_mesh { loadCubeInstancedMesh() };

    static constexpr std::uint8_t no_hover_index = 0xFF;
    std::uint8_t hovered_index = no_hover_index;

    std::vector<glm::vec3> rotation_axes;
    std::vector<glm::mat4> models;

    // View/projection related properties.
    static constexpr float camera_distance = 10.f;
    std::optional<glm::vec3> camera_velocity;
    DirtyProperty<glm::mat4> view;

    DirtyProperty<float> aspect { 1.f };
    DirtyProperty<float> fov { 45.f };
    DirtyProperty<glm::mat4> projection;

    OGLWrapper::Buffer<GL_UNIFORM_BUFFER, VpMatrix> vp_matrix_ubo { GL_DYNAMIC_DRAW };

    // Window event handlers.
    void onScrollCallback(OGLWrapper::GLFW::EventArg&, glm::dvec2 offset);
    void onCursorPosCallback(OGLWrapper::GLFW::EventArg&, glm::dvec2 position);
    void onKeyCallback(OGLWrapper::GLFW::EventArg&, int key, int scancode, int action, int mods);

    void recreateFramebuffer(glm::ivec2 size);

    // Render loop related functions.
    void update(float time_delta);
    void updateImGui(float time_delta);
    void draw() const;
    void drawImGui() const;
    void onRenderLoop(float time_delta) override;

    void initImGui();
    void initModels();

    static OGLWrapper::Helper::GpuMesh<VertexPNT> loadCubeMesh();
    static GpuInstancedMesh<VertexPNT, glm::mat4> loadCubeInstancedMesh();

public:
    AppWindow();
    ~AppWindow() override;
};
