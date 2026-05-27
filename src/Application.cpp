#include <Application.h>

#include <DepthMap.h>
#include <DepthMesh.h>
#include <ImageResource.h>
#include <WarpRenderer.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <filesystem>
#include <iostream>

#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <commdlg.h>

namespace {

// Small utility callbacks/helpers used by the window and UI layer.
void glfwErrorCallback(int error, const char* description) {
    std::cerr << "GLFW error " << error << ": " << description << "\n";
}

ImVec2 fitSize(float sourceW, float sourceH, float maxW, float maxH) {
    const float scale = std::min(maxW / sourceW, maxH / sourceH);
    return ImVec2(sourceW * scale, sourceH * scale);
}

std::string openImageFileDialog(const char* title) {
    char fileName[MAX_PATH] = {};

    OPENFILENAMEA dialog = {};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = nullptr;
    dialog.lpstrFilter = "Image Files (*.png;*.jpg;*.jpeg;*.bmp;*.tga)\0*.png;*.jpg;*.jpeg;*.bmp;*.tga\0All Files (*.*)\0*.*\0";
    dialog.lpstrFile = fileName;
    dialog.nMaxFile = MAX_PATH;
    dialog.lpstrTitle = title;
    dialog.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameA(&dialog) == TRUE) {
        return fileName;
    }
    return {};
}

// All tweakable UI values live in one place so the renderer, mesh builder,
// and depth editor can stay focused on their own jobs.
struct DemoSettings {
    std::string imagePath;
    std::string depthPath;

    float luminanceInfluence = 0.18f;
    int smoothingIterations = 2;
    bool invertDepth = false;
    float depthScale = 0.70f;
    float depthBias = -0.18f;
    float tearThreshold = 0.08f;

    int meshCols = 160;
    int meshRows = 120;
    CameraSettings camera;

    float brushRadius = 28.0f;
    float brushDepth = 0.85f;
    float brushStrength = 0.35f;
};

class Application {
public:
    // Create renderer resources and optionally preload an image path passed from main().
    bool init(const std::string& inputPath) {
        settings_.imagePath = inputPath;
        if (!renderer_.init()) {
            return false;
        }
        if (!settings_.imagePath.empty()) {
            loadImageFromUi();
        }
        return true;
    }

    void shutdown() {
        mesh_.destroy();
        depth_.destroyTexture();
        image_.destroyTexture();
        renderer_.shutdown();
    }

    void tick() {
        // All expensive updates are delayed until a setting actually changes.
        if (depth_.valid() && depth_.dirty()) {
            depth_.updatePreviewTexture();
        }
        if (meshDirty_ && inputsReadyForMesh()) {
            rebuildMesh();
        }
        if (sceneReady()) {
            renderer_.render(image_, mesh_, settings_.camera);
            if (exportResultRequested_) {
                exportCurrentResult();
            }
        }
    }

    void drawMainCanvas(int displayW, int displayH) const {
        // The warp result is drawn on the main window, not inside an ImGui image widget.
        if (sceneReady()) {
            renderer_.target().blitToDefaultFramebuffer(displayW, displayH);
        }
    }

    void drawUi(int displayH) {
        ImGui::SetNextWindowPos(ImVec2(16.0f, 16.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(430.0f, static_cast<float>(std::max(420, displayH - 32))), ImGuiCond_FirstUseEver);
        ImGui::Begin("Control Panel");
        ImGui::Text("RGB image + editable depth map -> textured proxy mesh -> real-time novel-view warp");

        drawInputControls();
        drawWarpControls();
        drawBrushControls();
        drawDebugViews();

        ImGui::End();
    }

private:
    // Fallback depth is only for quick testing. A real depth image usually gives better results.
    void regenerateDepth() {
        if (!image_.valid()) {
            status_ = "Load an RGB image first.";
            return;
        }
        depth_.generate(image_, settings_.luminanceInfluence, settings_.smoothingIterations, settings_.invertDepth);
        status_ = "Generated fallback depth from RGB image.";
        meshDirty_ = true;
    }

    void rebuildMesh() {
        if (!inputsReadyForMesh()) {
            return;
        }
        mesh_.rebuild(image_,
                      depth_,
                      settings_.meshCols,
                      settings_.meshRows,
                      settings_.depthScale,
                      settings_.depthBias,
                      settings_.tearThreshold);
        meshDirty_ = false;
    }

    void loadImageFromUi() {
        if (image_.loadWithTexture(settings_.imagePath)) {
            status_ = "Loaded RGB image.";
            meshDirty_ = depth_.matchesSize(image_.width(), image_.height());
            if (depth_.valid() && !depth_.matchesSize(image_.width(), image_.height())) {
                status_ = "Loaded RGB image. Depth dimensions do not match yet.";
            }
        }
    }

    void loadDepthFromUi() {
        ImageResource depthImage;
        if (!depthImage.loadCpuOnly(settings_.depthPath)) {
            status_ = "Failed to load depth image.";
            return;
        }

        depth_.loadFromImage(depthImage, settings_.invertDepth);
        depth_.updatePreviewTexture();
        status_ = "Loaded depth image.";

        if (image_.valid() && !depth_.matchesSize(image_.width(), image_.height())) {
            status_ = "Loaded depth image, but dimensions do not match RGB image.";
        }
        meshDirty_ = true;
    }

    bool sceneReady() const {
        // Rendering needs both inputs and a mesh that has already been uploaded.
        return image_.valid() && depth_.matchesSize(image_.width(), image_.height()) && mesh_.indexCount() > 0;
    }

    bool inputsReadyForMesh() const {
        // The RGB and depth maps must match pixel dimensions because UVs are shared.
        return image_.valid() && depth_.matchesSize(image_.width(), image_.height());
    }

    void markMeshDirtyIfInputsReady() {
        if (inputsReadyForMesh()) {
            meshDirty_ = true;
        }
    }

    void requestResultExport() {
        if (!sceneReady()) {
            status_ = "Load matching RGB/depth images before exporting.";
            return;
        }
        // The actual save happens after the next render pass, so the exported image matches the current view.
        exportResultRequested_ = true;
        status_ = "Exporting current render...";
    }

    void exportCurrentResult() {
        const std::string outputPath = "Output_Images/current_warp_result.png";
        std::filesystem::create_directories("Output_Images");

        if (renderer_.target().savePng(outputPath)) {
            status_ = "Exported current render to " + outputPath;
        } else {
            status_ = "Failed to export current render.";
        }
        exportResultRequested_ = false;
    }

    void drawInputControls() {
        if (!ImGui::CollapsingHeader("Input and Depth", ImGuiTreeNodeFlags_DefaultOpen)) {
            return;
        }

        // RGB image controls: typed path, manual load, or native Windows file dialog.
        char pathBuffer[512] = {};
        std::snprintf(pathBuffer, sizeof(pathBuffer), "%s", settings_.imagePath.c_str());
        ImGui::SetNextItemWidth(420.0f);
        if (ImGui::InputText("Image path", pathBuffer, sizeof(pathBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            settings_.imagePath = pathBuffer;
            loadImageFromUi();
        }
        ImGui::SameLine();
        if (ImGui::Button("Load")) {
            loadImageFromUi();
        }
        ImGui::SameLine();
        if (ImGui::Button("Browse image")) {
            const std::string selectedPath = openImageFileDialog("Select RGB image");
            if (!selectedPath.empty()) {
                settings_.imagePath = selectedPath;
                loadImageFromUi();
            }
        }

        // Depth image controls mirror the RGB path controls. The depth map is read as grayscale.
        char depthPathBuffer[512] = {};
        std::snprintf(depthPathBuffer, sizeof(depthPathBuffer), "%s", settings_.depthPath.c_str());
        ImGui::SetNextItemWidth(420.0f);
        if (ImGui::InputText("Depth path", depthPathBuffer, sizeof(depthPathBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
            settings_.depthPath = depthPathBuffer;
            loadDepthFromUi();
        }
        ImGui::SameLine();
        if (ImGui::Button("Load depth")) {
            loadDepthFromUi();
        }
        ImGui::SameLine();
        if (ImGui::Button("Browse depth")) {
            const std::string selectedPath = openImageFileDialog("Select depth image");
            if (!selectedPath.empty()) {
                settings_.depthPath = selectedPath;
                loadDepthFromUi();
            }
        }

        if (!status_.empty()) {
            ImGui::TextWrapped("%s", status_.c_str());
        }

        // These only affect the optional generated fallback depth map.
        ImGui::SliderFloat("Luminance influence", &settings_.luminanceInfluence, 0.0f, 0.75f);
        ImGui::SliderInt("Generation smoothing", &settings_.smoothingIterations, 0, 8);
        ImGui::Checkbox("Invert depth on load/generation", &settings_.invertDepth);
        if (ImGui::Button("Generate fallback depth")) {
            regenerateDepth();
        }
        ImGui::SameLine();
        if (ImGui::Button("Smooth current depth")) {
            depth_.smoothCurrent(1);
            markMeshDirtyIfInputsReady();
        }
        ImGui::SameLine();
        if (ImGui::Button("Export depth PNG")) {
            if (!depth_.savePreviewPng("Input_Images/exported_depth_preview.png")) {
                std::cerr << "Failed to export depth preview.\n";
            }
        }
    }

    void drawWarpControls() {
        if (!ImGui::CollapsingHeader("Warp and Mesh Controls", ImGuiTreeNodeFlags_DefaultOpen)) {
            return;
        }

        // Geometry controls rebuild the mesh because they change vertex positions or triangle removal.
        if (ImGui::SliderFloat("Depth scale", &settings_.depthScale, -1.25f, 1.25f)) markMeshDirtyIfInputsReady();
        if (ImGui::SliderFloat("Depth bias", &settings_.depthBias, -1.0f, 1.0f)) markMeshDirtyIfInputsReady();
        if (ImGui::SliderFloat("Depth tear threshold", &settings_.tearThreshold, 0.0f, 1.0f)) markMeshDirtyIfInputsReady();
        if (ImGui::SliderInt("Mesh columns", &settings_.meshCols, 16, 320)) markMeshDirtyIfInputsReady();
        if (ImGui::SliderInt("Mesh rows", &settings_.meshRows, 16, 240)) markMeshDirtyIfInputsReady();
        // Camera/background controls affect rendering only, so they do not need a mesh rebuild.
        ImGui::ColorEdit3("Background color", glm::value_ptr(settings_.camera.bgColor));
        ImGui::Checkbox("Wireframe", &settings_.camera.wireframe);
        ImGui::SameLine();
        if (ImGui::Button("Export result PNG")) {
            requestResultExport();
        }
        ImGui::Separator();
        ImGui::SliderFloat("Yaw", &settings_.camera.yaw, -45.0f, 45.0f);
        ImGui::SliderFloat("Pitch", &settings_.camera.pitch, -35.0f, 35.0f);
        ImGui::SliderFloat("Zoom", &settings_.camera.zoom, 1.0f, 5.0f);
        ImGui::SliderFloat2("Pan", glm::value_ptr(settings_.camera.pan), -0.8f, 0.8f);
        ImGui::SliderFloat("Perspective/FOV", &settings_.camera.fov, 18.0f, 75.0f);
    }

    void drawBrushControls() {
        if (!ImGui::CollapsingHeader("Depth Brush", ImGuiTreeNodeFlags_DefaultOpen)) {
            return;
        }

        ImGui::SliderFloat("Brush radius", &settings_.brushRadius, 2.0f, 120.0f);
        ImGui::SliderFloat("Brush target depth", &settings_.brushDepth, 0.0f, 1.0f);
        ImGui::SliderFloat("Brush strength", &settings_.brushStrength, 0.01f, 1.0f);
        ImGui::Text("Left-drag on depth preview paints. Right-drag smooths locally.");
    }

    void drawDebugViews() {
        const float panelW = ImGui::GetContentRegionAvail().x;
        const float itemW = std::max(170.0f, (panelW - 12.0f) / 2.0f);
        const float itemH = 230.0f;

        ImGui::Separator();
        ImGui::Text("Input Images");
        ImGui::Columns(2, "debug_views", false);

        // The debug previews are deliberately kept in the UI; the actual warp appears on the main canvas.
        ImGui::Text("Original image");
        if (image_.valid()) {
            const ImVec2 imagePreviewSize = fitSize(static_cast<float>(image_.width()), static_cast<float>(image_.height()), itemW, itemH);
            ImGui::Image(static_cast<ImTextureID>(image_.texture()), imagePreviewSize);
        } else {
            ImGui::TextWrapped("No RGB image loaded.");
        }
        ImGui::NextColumn();

        ImGui::Text("Editable depth map");
        if (depth_.valid()) {
            const ImVec2 depthPreviewSize = fitSize(static_cast<float>(depth_.width()), static_cast<float>(depth_.height()), itemW, itemH);
            ImVec2 depthPos = ImGui::GetCursorScreenPos();

            // InvisibleButton makes the preview an active drag target, so painting does not move the ImGui window.
            ImGui::InvisibleButton("depth_paint_canvas",
                                   depthPreviewSize,
                                   ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
            ImGui::GetWindowDrawList()->AddImage(static_cast<ImTextureID>(depth_.texture()),
                                                 depthPos,
                                                 ImVec2(depthPos.x + depthPreviewSize.x, depthPos.y + depthPreviewSize.y));

            const bool paintingActive = ImGui::IsItemHovered() || ImGui::IsItemActive();
            if (paintingActive && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                paintDepthFromUi(depthPos, depthPreviewSize, false);
            }
            if (paintingActive && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                paintDepthFromUi(depthPos, depthPreviewSize, true);
            }
        } else {
            ImGui::TextWrapped("No depth image loaded.");
        }
        ImGui::Columns(1);
    }

    void paintDepthFromUi(ImVec2 imageMin, ImVec2 imageSize, bool smoothMode) {
        ImGuiIO& io = ImGui::GetIO();
        const float localX = io.MousePos.x - imageMin.x;
        const float localY = io.MousePos.y - imageMin.y;
        if (localX < 0.0f || localY < 0.0f || localX >= imageSize.x || localY >= imageSize.y) {
            return;
        }

        // Convert from preview-window coordinates back to full-resolution depth-map coordinates.
        const float imgX = (localX / imageSize.x) * static_cast<float>(depth_.width());
        const float imgY = (localY / imageSize.y) * static_cast<float>(depth_.height());
        depth_.paintAt(imgX, imgY, settings_.brushRadius, settings_.brushDepth, settings_.brushStrength, smoothMode);
        markMeshDirtyIfInputsReady();
    }

    DemoSettings settings_;
    ImageResource image_;
    DepthMap depth_;
    DepthMesh mesh_;
    WarpRenderer renderer_;
    std::string status_;
    bool meshDirty_ = true;
    bool exportResultRequested_ = false;
};

}  // namespace

int runInteractiveApp(const std::string& inputPath) {
    // Window/OpenGL setup.
    glfwSetErrorCallback(glfwErrorCallback);
    if (!glfwInit()) {
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1440, 900, "IBR Viewer", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed to initialize GLAD.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // ImGui setup.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    Application app;
    if (!app.init(inputPath)) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Main loop: update data, draw the warped scene, then draw the UI overlay.
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        app.tick();

        int displayW = 0;
        int displayH = 0;
        glfwGetFramebufferSize(window, &displayW, &displayH);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, displayW, displayH);
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.08f, 0.085f, 0.095f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        app.drawMainCanvas(displayW, displayH);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        app.drawUi(displayH);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup order mirrors setup order.
    app.shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
