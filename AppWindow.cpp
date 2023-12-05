//
// Created by gomkyung2 on 11/26/23.
//

#include "AppWindow.hpp"

#include <fstream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <ImGuizmo.h>

#include <OGLWrapper/Helper/Camera.hpp>

#include <glm/gtc/random.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <range/v3/view.hpp>
#include <range/v3/range/conversion.hpp>

template <>
struct OGLWrapper::Helper::VertexAttributes<glm::mat4> {
    std::array<GLuint, 4> columns;

    void setVertexAttribArrays() const {
        for (auto [idx, attribute] : columns | ranges::views::enumerate) {
            glEnableVertexAttribArray(attribute);
            glVertexAttribPointer(
                attribute,
                4,
                GL_FLOAT,
                GL_FALSE,
                sizeof(glm::mat4),
                reinterpret_cast<const void *>(sizeof(glm::vec4) * idx));
        }
    }

    void setVertexAttribDivisors() const {
        for (GLuint attribute : columns) {
            glVertexAttribDivisor(attribute, 1);
        }
    }
};

void AppWindow::onScrollCallback(OGLWrapper::GLFW::EventArg&, glm::dvec2 offset) {
    ImGuiIO& io = ImGui::GetIO();
    io.AddMouseWheelEvent(offset.x, offset.y);
    if (io.WantCaptureMouse){
        return;
    }

    fov = std::clamp(fov.value() + static_cast<float>(offset.y), 15.f, 150.f);
}

void AppWindow::onCursorPosCallback(OGLWrapper::GLFW::EventArg&, glm::dvec2 position) {
    ImGuiIO &io = ImGui::GetIO();
    io.AddMousePosEvent(position.x, position.y);
    if (io.WantCaptureMouse){
        return;
    }

    // In parameter `position` is in window coordinates, but we need to convert it to framebuffer coordinates.
    const glm::dvec2 framebuffer_size = getFramebufferSize();
    const glm::dvec2 scale = framebuffer_size / glm::dvec2(getSize());
    const glm::ivec2 framebuffer_cursor_position = scale * position;

    // Since OpenGL has bottom-left origined coordinate, y-axis must be inverted.
    const glm::ivec2 opengl_cursor_position { framebuffer_cursor_position.x, framebuffer_size.y - framebuffer_cursor_position.y };

    // Read stencil value at the cursor position, store into hovered_index.
    framebuffer.bind();
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glReadPixels(opengl_cursor_position.x, opengl_cursor_position.y, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_BYTE, &hovered_index);
}

void AppWindow::onKeyCallback(OGLWrapper::GLFW::EventArg&, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        const glm::mat4 inv_view = inverse(view.value());
        const glm::vec3 front = OGLWrapper::Helper::Camera::getFront(inv_view);
        const glm::vec3 right = OGLWrapper::Helper::Camera::getRight(inv_view);

        constexpr float speed = 2.f;
        camera_velocity = [&]() -> std::optional<glm::vec3> {
            switch (key) {
                case GLFW_KEY_W: return speed * front;
                case GLFW_KEY_A: return speed * -right;
                case GLFW_KEY_S: return speed * -front;
                case GLFW_KEY_D: return speed * right;
                default: return std::nullopt;
            }
        }();
    }
    else if (action == GLFW_RELEASE && camera_velocity) {
        camera_velocity = std::nullopt;
    }
}

void AppWindow::recreateFramebuffer(glm::ivec2 size) {
    color_attachment = [&size]() {
        OGLWrapper::Texture<GL_TEXTURE_2D> texture{};

        glActiveTexture(GL_TEXTURE2);
        texture.bind();

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, size.x, size.y, 0, GL_RGB, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        return texture;
    }();
    framebuffer.attachTexture(GL_COLOR_ATTACHMENT0, color_attachment);

    depth_attachment = [&size]() {
        OGLWrapper::Renderbuffer renderbuffer{};
        renderbuffer.bind();
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size.x, size.y);

        return renderbuffer;
    }();
    framebuffer.attachRenderbuffer(GL_DEPTH_ATTACHMENT, depth_attachment);

    instance_id_attachment = [&size]() {
        OGLWrapper::Texture<GL_TEXTURE_2D> texture{};

        glActiveTexture(GL_TEXTURE3);
        texture.bind();

        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, size.x, size.y, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        return texture;
    }();
    framebuffer.attachTexture(GL_COLOR_ATTACHMENT1, instance_id_attachment);

    assert(framebuffer.isComplete());
    glViewport(0, 0, size.x, size.y);

    // color_attachment and instance_id_attachment will be used in fragment shader.
    constexpr std::array<GLuint, 2> attachments { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(attachments.size(), attachments.data());
}

void AppWindow::update(float time_delta) {
    // If camera has velocity, i.e. user pressed WASD, `view` should be also updated.
    if (camera_velocity) {
        glm::mat4 inv_view = inverse(view.value());
        inv_view[3] += glm::vec4 { camera_velocity.value() * time_delta, 0.f };
        view = inverse(inv_view);
    }

    // Rotate models along their rotation axis.
    for (auto &&[model, rotation_axis] : ranges::views::zip(models, rotation_axes)) {
        model = rotate(model, time_delta, rotation_axis);
    }
    std::get<0>(cube_instanced_mesh.instance_buffers)
        .getSubBuffer()
        .store(models);

    // If either `fov` or `aspect` changed, `projection` should be also updated.
    DirtyPropertyHelper::clean([&](float fov, float aspect) {
        projection = glm::perspective(glm::radians(fov), aspect, 1e-2f, 100.f);
    }, fov, aspect);

    // If either `view` or `projection` changed, shader's `projection_view` should be also updated.
    DirtyPropertyHelper::clean([&](const glm::mat4 &view, const glm::mat4 &projection) {
        const glm::mat4 inv_view = inverse(view);
        const VpMatrix vp_matrix {
            .projection_view = projection * view,
            .view_pos = OGLWrapper::Helper::Camera::getPosition(inv_view)
        };
        vp_matrix_ubo.getSubBuffer().store(std::span{ &vp_matrix, 1 });
    }, view, projection);
}

void AppWindow::updateImGui(float time_delta) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGuizmo::BeginFrame();

    // Clone the view matrix and pass it to ImGuizmo.
    glm::mat4 new_view = view.value();
    ImGuizmo::ViewManipulate(
        value_ptr(new_view),
        camera_distance,
        ImVec2(0, 0),
        ImVec2(64, 64),
        0x10101010);

    // Check if ImGuizmo modified the view matrix. If so, update the property.
    if (new_view != view.value()) {
        view = new_view;
    }

    ImGui::Render();
}

void AppWindow::draw() const {
    // Pass 1: draw instanced cubes into `framebuffer`.
    // Each mesh's instance id will be also recorded in `instance_id_attachment`.
    framebuffer.bind();
    // Clear color_attachment and depth_attachment.
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    // Clear instance_id_attachment.
    constexpr GLuint no_hover_index_color = 0xFF;
    glClearBufferuiv(GL_COLOR, 1, &no_hover_index_color);
    // Use program and draw instanced mesh.
    primary_instanced_program.use();
    cube_instanced_mesh.draw(125);

    // Pass 2: blit framebuffer into default framebuffer.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer.handle);
    glReadBuffer(GL_COLOR_ATTACHMENT0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    constexpr std::array<GLenum, 1> attachments { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(attachments.size(), attachments.data());

    const glm::ivec2 framebuffer_size = getFramebufferSize();
    glBlitFramebuffer(
        0, 0, framebuffer_size.x, framebuffer_size.y,
        0, 0, framebuffer_size.x, framebuffer_size.y,
        GL_COLOR_BUFFER_BIT, GL_NEAREST);

    if (hovered_index != no_hover_index) {
        // If there is a mesh under the cursor (retrieved by stencil test), draw it with slightly scaled model.
        constexpr float scale_factor = 1.05f;

        assert(hovered_index < models.size());
        const glm::mat4 scaled_model = scale(models[hovered_index], glm::vec3 { scale_factor });

        glDisable(GL_DEPTH_TEST);
        outliner_instanced_program.use();
        glUniform1ui(outliner_instanced_program.getUniformLocation("instance_id"), hovered_index);
        glUniformMatrix4fv(outliner_instanced_program.getUniformLocation("model"), 1, GL_FALSE, value_ptr(scaled_model));

        cube_mesh.draw();

        // Settings should be restored for next render loop.
        glEnable(GL_DEPTH_TEST);
    }
}

void AppWindow::drawImGui() const {
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void AppWindow::onRenderLoop(float time_delta) {
    update(time_delta);
    updateImGui(time_delta);
    draw();
    drawImGui();
}

void AppWindow::initImGui() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(base, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void AppWindow::initModels() {
    // Each meshes are positioned at the 3d grid of [-2, -1, 0, 1, 2]^3.
    constexpr auto linspace = ranges::views::linear_distribute(-2.f, 2.f, 5);

    static constexpr glm::mat4 identity = glm::identity<glm::mat4>();
    models =
        ranges::views::cartesian_product(linspace, linspace, linspace)
        | ranges::views::transform([](auto &&xyz) -> glm::mat4 {
            const glm::vec3 offset = std::make_from_tuple<glm::vec3>(xyz);
            return translate(identity, offset);
        })
        | ranges::to_vector;

    rotation_axes =
        ranges::views::generate_n([]() -> glm::vec3 { return glm::sphericalRand(1.f); }, models.size())
        | ranges::to_vector;
}

OGLWrapper::Helper::GpuMesh<VertexPNT> AppWindow::loadCubeMesh() {
    // TODO: use proper format (e.g. .obj)
    std::ifstream cube_file { "assets/models/cube.txt" };
    assert(cube_file.is_open());

    std::vector<VertexPNT> vertices { std::istream_iterator<VertexPNT> { cube_file }, {} };
    return OGLWrapper::Helper::Mesh { std::move(vertices) }
        .transferToGpu({ 0, 1, 2 }, GL_STATIC_DRAW);
}

GpuInstancedMesh<VertexPNT, glm::mat4> AppWindow::loadCubeInstancedMesh() {
    std::ifstream cube_file { "assets/models/cube.txt" };
    assert(cube_file.is_open());

    OGLWrapper::VertexArray vao{};
    vao.bind();

    OGLWrapper::Buffer<GL_ARRAY_BUFFER, VertexPNT> vbo { GL_STATIC_DRAW };
    std::vector<VertexPNT> vertices { std::istream_iterator<VertexPNT> { cube_file }, {} };
    vbo.store(vertices);

    OGLWrapper::Helper::VertexAttributes<VertexPNT>{ 0, 1, 2 }
        .setVertexAttribArrays();

    OGLWrapper::Buffer<GL_ARRAY_BUFFER, glm::mat4> ibo { GL_STREAM_DRAW };
    ibo.reserve(125 /* models.size() */);

    constexpr OGLWrapper::Helper::VertexAttributes<glm::mat4> instance_attributes { 3, 4, 5, 6 };
    instance_attributes.setVertexAttribArrays();
    instance_attributes.setVertexAttribDivisors();

    return { .vertex_array = std::move(vao),
        .vertex_buffer = std::move(vbo),
        .instance_buffers = std::move(ibo) };
}

AppWindow::AppWindow() : Window { 640, 640, "Mouse picking", {} },
                         view { lookAt(
                             camera_distance * normalize(glm::vec3 { 1.f }),
                             glm::vec3 { 0.f },
                             glm::vec3 { 0.f, 1.f, 0.f }) }
{
    initImGui();
    initModels();

    framebuffer_size_callback.append([this](OGLWrapper::GLFW::EventArg&, glm::ivec2) {
        // When framebuffer size changed, aspect ratio should be also updated.
        aspect = getFramebufferAspectRatio();
    });
    framebuffer_size_callback.append([this](OGLWrapper::GLFW::EventArg&, glm::ivec2 size) {
        // When using instancing (using multiple fbos), user-made fbo's attachment size should be also updated.
        recreateFramebuffer(size);
    });
    scroll_callback.append(std::bind_front(&AppWindow::onScrollCallback, this));
    cursor_pos_callback.append(std::bind_front(&AppWindow::onCursorPosCallback, this));
    key_callback.append(std::bind_front(&AppWindow::onKeyCallback, this));

    // Configure framebuffer.
    recreateFramebuffer(getFramebufferSize());

    // Set active textures.
    wood.setTexture(GL_TEXTURE0, GL_TEXTURE1);
    primary_instanced_program.pendUniforms([&]() {
        glUniform1i(primary_instanced_program.getUniformLocation("diffuse_map"), 0);
        glUniform1i(primary_instanced_program.getUniformLocation("specular_map"), 1);
    });
    outliner_instanced_program.pendUniforms([&]() {
        glUniform1i(outliner_instanced_program.getUniformLocation("instance_id_texture"), 3);
    });

    // Set VpMatrix.
    vp_matrix_ubo.reserve(1);
    OGLWrapper::Program::setUniformBlockBindings("VpMatrix", 0,
        primary_instanced_program, outliner_instanced_program);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, vp_matrix_ubo.handle);

    // Enable OpenGL features.
    glEnable(GL_CULL_FACE);
}

AppWindow::~AppWindow() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
