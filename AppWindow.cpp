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

void AppWindow::onFramebufferSizeCallback(OGLWrapper::GLFW::EventArg&, glm::ivec2 size) {
    aspect = getFramebufferAspectRatio();
}

void AppWindow::onScrollCallback(OGLWrapper::GLFW::EventArg&, glm::dvec2 offset) {
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
    glReadPixels(opengl_cursor_position.x, opengl_cursor_position.y, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, &hovered_index);
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

    // If either `fov` or `aspect` changed, `projection` should be also updated.
    DirtyPropertyHelper::clean([&](float fov, float aspect) {
        projection = glm::perspective(glm::radians(fov), aspect, 1e-2f, 100.f);
    }, fov, aspect);

    // If either `view` or `projection` changed, shader's `projection_view` should be also updated.
    DirtyPropertyHelper::clean([&](const glm::mat4 &view, const glm::mat4 &projection) {
        const glm::mat4 inv_view = inverse(view);
        const glm::vec3 view_pos = OGLWrapper::Helper::Camera::getPosition(inv_view);
        const glm::mat4 projection_view = projection * view;

        primary_program.pendUniforms([&, projection_view, view_pos]() {
            glUniformMatrix4fv(primary_program.getUniformLocation("vp_matrix.projection_view"), 1, GL_FALSE, value_ptr(projection_view));
            glUniform3fv(primary_program.getUniformLocation("vp_matrix.view_pos"), 1, value_ptr(view_pos));
        });
        outliner_program.pendUniforms([&, projection_view, view_pos]() {
            glUniformMatrix4fv(outliner_program.getUniformLocation("vp_matrix.projection_view"), 1, GL_FALSE, value_ptr(projection_view));
            glUniform3fv(outliner_program.getUniformLocation("vp_matrix.view_pos"), 1, value_ptr(view_pos));
        });
    }, view, projection);
}

void AppWindow::updateImGui(float time_delta) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    constexpr std::size_t num_frame_times = 1 << 8;
    static std::vector fps_record(num_frame_times, 0.f);
    static float fps_average = 0.f;
    static std::size_t current_idx = 0;

    fps_record[current_idx] = 1.f / time_delta;
    if (++current_idx == fps_record.size()) {
        current_idx = 0;
        fps_average = std::reduce(fps_record.cbegin(), fps_record.cend()) / fps_record.size();
    }

    ImGui::Begin("Inspector");

    if (static bool vsync = true; ImGui::Checkbox("Enable V-Sync", &vsync)) {
        glfwSwapInterval(vsync ? 1 : 0);
    }
    ImGui::PlotLines("FPS", fps_record.data(), fps_record.size());
    ImGui::Text("Average FPS: %.1f", fps_average);

    if (static glm::vec3 outline_color { 1.f, 0.5f, 0.2f }; ImGui::ColorEdit3("Outline color", value_ptr(outline_color))) {
        outliner_program.pendUniforms([& /* outline_color is in static lifetime: no need to value capture */]() {
            glUniform3fv(outliner_program.getUniformLocation("color"), 1, value_ptr(outline_color));
        });
    }

    ImGui::End();

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    primary_program.use();
    for (auto &&[idx, model] : models | ranges::views::enumerate) {
        glUniformMatrix4fv(primary_program.getUniformLocation("model"), 1, GL_FALSE, value_ptr(model));

        const glm::mat4 inv_model = inverse(model);
        glUniformMatrix4fv(primary_program.getUniformLocation("inv_model"), 1, GL_FALSE, value_ptr(inv_model));

        glStencilFunc(GL_ALWAYS, static_cast<GLint>(idx), 0xFF);
        cube_mesh.draw();
    }

    if (hovered_index != no_hover_index) {
        // If there is a mesh under the cursor (retrieved by stencil test), draw it with slightly scaled model.
        outliner_program.use();

        // The stencil function set to GL_NOTEQUAL with reference value = hovered_index will pass only scaled fragments
        // (i.e. outline) of the mesh.
        glStencilFunc(GL_NOTEQUAL, static_cast<GLint>(hovered_index), no_hover_index);
        glStencilMask(0x00);
        glDisable(GL_DEPTH_TEST);

        constexpr float scale_factor = 1.05f;

        assert(hovered_index < models.size());
        const glm::mat4 scaled_model = scale(models[hovered_index], glm::vec3 { scale_factor });
        glUniformMatrix4fv(outliner_program.getUniformLocation("model"), 1, GL_FALSE, value_ptr(scaled_model));

        cube_mesh.draw();

        // Settings should be restored for next render loop.
        glStencilMask(0xFF);
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
    constexpr auto linspace = ranges::views::linear_distribute(-2.f, 2.f, num_cube_in_side);

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

    std::vector<VertexPNT> vertices;
    for (VertexPNT vertex; cube_file >> vertex;) {
        vertices.push_back(vertex);
    }

    return OGLWrapper::Helper::Mesh { std::move(vertices) }
        .transferToGpu({ 0, 1, 2 }, GL_STATIC_DRAW);
}

AppWindow::AppWindow() : Window { 640, 640, "Mouse picking", {} },
                         cube_mesh { loadCubeMesh() },
                         view { lookAt(
                             camera_distance * normalize(glm::vec3 { 1.f }),
                             glm::vec3 { 0.f },
                             glm::vec3 { 0.f, 1.f, 0.f }) }
{
    initImGui();
    initModels();

    framebuffer_size_callback.append(std::bind_front(&AppWindow::onFramebufferSizeCallback, this));
    scroll_callback.append(std::bind_front(&AppWindow::onScrollCallback, this));
    cursor_pos_callback.append(std::bind_front(&AppWindow::onCursorPosCallback, this));
    key_callback.append(std::bind_front(&AppWindow::onKeyCallback, this));

    // Set texture.
    primary_program.pendUniforms([&]() {
        glUniform1i(primary_program.getUniformLocation("diffuse_map"), 0);
        glUniform1i(primary_program.getUniformLocation("specular_map"), 1);
    });
    wood.setTexture(GL_TEXTURE0, GL_TEXTURE1);

    // Enable OpenGL features.
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);

    // At first, stencil buffer will filled with `no_hover_index`.
    // At draw call, stencil buffer will be filled with `idx` if the fragment is drawn and depth test is passed.
    // After that, we can retrieve the index of the mesh under the cursor by reading stencil buffer.
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);
    glClearStencil(no_hover_index);
}

AppWindow::~AppWindow() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
