#include <Application.h>

#include <DepthMap.h>
#include <DepthMesh.h>
#include <ImageResource.h>
#include <LightFieldSlab.h>
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
#include <shobjidl.h>

namespace {

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

std::string openFolderDialog(const char* title) {
    std::string result;
    HRESULT hrInit = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
    IFileOpenDialog *pfd = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd)))) {
        DWORD dwOptions;
        if (SUCCEEDED(pfd->GetOptions(&dwOptions))) {
            pfd->SetOptions(dwOptions | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
        }
        if (SUCCEEDED(pfd->Show(NULL))) {
            IShellItem *psi;
            if (SUCCEEDED(pfd->GetResult(&psi))) {
                PWSTR pszPath;
                if (SUCCEEDED(psi->GetDisplayName(SIGDN_FILESYSPATH, &pszPath))) {
                    char path[MAX_PATH];
                    WideCharToMultiByte(CP_UTF8, 0, pszPath, -1, path, MAX_PATH, NULL, NULL);
                    result = path;
                    CoTaskMemFree(pszPath);
                }
                psi->Release();
            }
        }
        pfd->Release();
    }
    if (SUCCEEDED(hrInit)) {
        CoUninitialize();
    }
    std::replace(result.begin(), result.end(), '\\', '/');
    return result;
}

const char* interpolationModeNames[] = { "Nearest", "Bilinear", "Quadrilinear" };

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

    std::string bundlePath = "Input_Images/Stanford_Dragon";
    int interpolationMode = 1; // 0=Nearest, 1=Bilinear, 2=Quadrilinear
    bool multiSlabActive = false;
    float backgroundCutoff = 0.12f; // skip background triangles (depth < cutoff)
    bool useVQ = false; // toggle for VQ compression
};

class Application {
public:
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
        for (auto& slab : slabs_) {
            slab.destroy();
        }
        slabs_.clear();

        mesh_.destroy();
        depth_.destroyTexture();
        image_.destroyTexture();
        renderer_.shutdown();
    }

    void tick() {
        if (settings_.multiSlabActive && !slabs_.empty()) {
            if (multiSlabMeshDirty_) {
                rebuildAllSlabMeshes();
            }
            if (anySlabReady()) {
                InterpolationMode mode = static_cast<InterpolationMode>(settings_.interpolationMode);
                renderer_.renderMultiSlab(slabs_, settings_.camera, mode);
                if (exportResultRequested_) {
                    exportCurrentResult();
                }
            }
        } else {
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
    }

    void drawMainCanvas(int displayW, int displayH) const {
        bool ready = settings_.multiSlabActive ? anySlabReady() : sceneReady();
        if (ready) {
            renderer_.target().blitToDefaultFramebuffer(displayW, displayH);
        }
    }

    void drawUi(int displayH) {
        ImGui::SetNextWindowPos(ImVec2(16.0f, 16.0f), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(430.0f, static_cast<float>(std::max(420, displayH - 32))), ImGuiCond_FirstUseEver);
        ImGui::Begin("Control Panel");
        ImGui::Text("RGB image + editable depth map -> textured proxy mesh -> real-time novel-view warp");

        drawMultiSlabControls();
        drawInputControls();
        drawWarpControls();
        drawBrushControls();
        drawDebugViews();

        ImGui::End();
    }

private:
    // ========================================================================
    // ========================================================================

    void loadBundleScene(const std::string& bundleDir) {
        for (auto& slab : slabs_) {
            slab.destroy();
        }
        slabs_.clear();

        struct SlabDef {
            std::string rgbFile;
            std::string depthFile;
            SlabOrientation orient;
            std::string label;
        };

        const SlabDef defs[] = {
            { bundleDir + "/front.png",  bundleDir + "/front_depth.png",
              {   0.0f,   0.0f }, "Front" },
            { bundleDir + "/back.png",   bundleDir + "/back_depth.png",
              { 180.0f,   0.0f }, "Back" },
            { bundleDir + "/left.png",   bundleDir + "/left_depth.png",
              { -90.0f,   0.0f }, "Left" },
            { bundleDir + "/right.png",  bundleDir + "/right_depth.png",
              {  90.0f,   0.0f }, "Right" },
        };

        int loadedCount = 0;
        for (const auto& def : defs) {
            LightFieldSlab slab;
            if (slab.loadFromFiles(def.rgbFile, def.depthFile, def.orient, def.label,
                                   settings_.invertDepth)) {
                slabs_.push_back(std::move(slab));
                ++loadedCount;
            }
        }

        if (loadedCount > 0) {
            settings_.multiSlabActive = true;
            multiSlabMeshDirty_ = true;
            settings_.camera.yaw = 0.0f;
            settings_.camera.pitch = 0.0f;
            settings_.camera.zoom = 2.5f;
            status_ = "Loaded " + std::to_string(loadedCount) + " slabs from bundle.";
        } else {
            status_ = "Failed to load any slabs from " + bundleDir;
        }
    }

    void rebuildAllSlabMeshes() {
        InterpolationMode mode = static_cast<InterpolationMode>(settings_.interpolationMode);
        for (auto& slab : slabs_) {
            slab.rebuildMesh(settings_.meshCols, settings_.meshRows,
                             settings_.depthScale, settings_.depthBias,
                             settings_.tearThreshold, settings_.backgroundCutoff,
                             mode);
        }
        multiSlabMeshDirty_ = false;
    }

    bool anySlabReady() const {
        for (const auto& slab : slabs_) {
            if (slab.meshReady()) return true;
        }
        return false;
    }

    void drawMultiSlabControls() {
        if (!ImGui::CollapsingHeader("Multi-Slab Light Field", ImGuiTreeNodeFlags_DefaultOpen)) {
            return;
        }

        char bundlePathBuffer[512] = {};
        std::snprintf(bundlePathBuffer, sizeof(bundlePathBuffer), "%s", settings_.bundlePath.c_str());
        ImGui::SetNextItemWidth(250.0f);
        if (ImGui::InputText("Bundle Path", bundlePathBuffer, sizeof(bundlePathBuffer))) {
            settings_.bundlePath = bundlePathBuffer;
        }
        ImGui::SameLine();
        if (ImGui::Button("Load Bundle")) {
            loadBundleScene(settings_.bundlePath);
        }
        ImGui::SameLine();
        if (ImGui::Button("Browse Bundle")) {
            const std::string selectedPath = openFolderDialog("Select Bundle Folder");
            if (!selectedPath.empty()) {
                settings_.bundlePath = selectedPath;
                loadBundleScene(settings_.bundlePath);
            }
        }

        if (ImGui::Checkbox("Use VQ Compression", &settings_.useVQ)) {
            if (settings_.useVQ) {
                status_ = "Compressing textures... please wait.";
                for (auto& slab : slabs_) {
                    if (slab.image().compressVQ()) {
                        slab.image().uploadTextureVQ();
                    }
                }
                status_ = "VQ Compression enabled.";
            } else {
                for (auto& slab : slabs_) {
                    slab.image().disableVQ();
                }
                status_ = "VQ Compression disabled.";
            }
        }

        if (!slabs_.empty()) {
            ImGui::SameLine();
            if (ImGui::Button("Unload Multi-Slab")) {
                for (auto& slab : slabs_) slab.destroy();
                slabs_.clear();
                settings_.multiSlabActive = false;
                status_ = "Unloaded all slabs.";
            }
        }
        
        ImGui::Combo("Interpolation", &settings_.interpolationMode, interpolationModeNames, 3);

        if (settings_.multiSlabActive && !slabs_.empty()) {
            ImGui::Text("Active slabs: %d", static_cast<int>(slabs_.size()));

            SlabSelection sel = selectClosestSlabs(slabs_, settings_.camera.yaw, settings_.camera.pitch);
            if (sel.primaryIdx >= 0) {
                ImGui::Text("Primary:   [%s] (%.1f deg)",
                            slabs_[sel.primaryIdx].label().c_str(),
                            slabs_[sel.primaryIdx].angularDistance(settings_.camera.yaw, settings_.camera.pitch));
                if (sel.secondaryIdx >= 0) {
                    ImGui::Text("Secondary: [%s] (%.1f deg)  blend=%.2f",
                                slabs_[sel.secondaryIdx].label().c_str(),
                                slabs_[sel.secondaryIdx].angularDistance(settings_.camera.yaw, settings_.camera.pitch),
                                sel.blendWeight);
                }
            }

            if (ImGui::TreeNode("Slab Details")) {
                for (int i = 0; i < static_cast<int>(slabs_.size()); ++i) {
                    const auto& s = slabs_[i];
                    ImGui::BulletText("[%s] yaw=%.0f pitch=%.0f  %dx%d  tris=%d",
                                     s.label().c_str(),
                                     s.orientation().yaw, s.orientation().pitch,
                                     s.image().width(), s.image().height(),
                                     s.mesh().indexCount() / 3);
                }
                ImGui::TreePop();
            }
        }
    }

    // ========================================================================
    // ========================================================================

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
        return image_.valid() && depth_.matchesSize(image_.width(), image_.height()) && mesh_.indexCount() > 0;
    }

    bool inputsReadyForMesh() const {
        return image_.valid() && depth_.matchesSize(image_.width(), image_.height());
    }

    void markMeshDirtyIfInputsReady() {
        if (settings_.multiSlabActive) {
            multiSlabMeshDirty_ = true;
        } else if (inputsReadyForMesh()) {
            meshDirty_ = true;
        }
    }

    void requestResultExport() {
        bool ready = settings_.multiSlabActive ? anySlabReady() : sceneReady();
        if (!ready) {
            status_ = "Load matching RGB/depth images before exporting.";
            return;
        }
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
        if (settings_.multiSlabActive) {
            return;
        }

        if (!ImGui::CollapsingHeader("Input and Depth", ImGuiTreeNodeFlags_DefaultOpen)) {
            return;
        }

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

        if (ImGui::SliderFloat("Depth scale", &settings_.depthScale, -1.25f, 1.25f)) markMeshDirtyIfInputsReady();
        if (ImGui::SliderFloat("Depth bias", &settings_.depthBias, -1.0f, 1.0f)) markMeshDirtyIfInputsReady();
        if (ImGui::SliderFloat("Depth tear threshold", &settings_.tearThreshold, 0.0f, 1.0f)) markMeshDirtyIfInputsReady();
        if (ImGui::SliderFloat("Background cutoff", &settings_.backgroundCutoff, 0.0f, 1.0f, "%.3f")) markMeshDirtyIfInputsReady();
        if (ImGui::SliderInt("Mesh columns", &settings_.meshCols, 16, 320)) markMeshDirtyIfInputsReady();
        if (ImGui::SliderInt("Mesh rows", &settings_.meshRows, 16, 240)) markMeshDirtyIfInputsReady();
        ImGui::ColorEdit3("Background color", glm::value_ptr(settings_.camera.bgColor));
        ImGui::Checkbox("Wireframe", &settings_.camera.wireframe);
        ImGui::SameLine();
        if (ImGui::Button("Export result PNG")) {
            requestResultExport();
        }
        ImGui::Separator();

        if (settings_.multiSlabActive) {
            ImGui::SliderFloat("Yaw",   &settings_.camera.yaw,   -180.0f, 180.0f);
            ImGui::SliderFloat("Pitch", &settings_.camera.pitch,  -90.0f,  90.0f);
        } else {
            ImGui::SliderFloat("Yaw",   &settings_.camera.yaw,    -45.0f,  45.0f);
            ImGui::SliderFloat("Pitch", &settings_.camera.pitch,   -35.0f,  35.0f);
        }
        ImGui::SliderFloat("Zoom", &settings_.camera.zoom, 1.0f, 5.0f);
        ImGui::SliderFloat2("Pan", glm::value_ptr(settings_.camera.pan), -0.8f, 0.8f);
        ImGui::SliderFloat("Perspective/FOV", &settings_.camera.fov, 18.0f, 75.0f);

        if (!status_.empty()) {
            ImGui::TextWrapped("%s", status_.c_str());
        }
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

        if (settings_.multiSlabActive && !slabs_.empty()) {
            ImGui::Text("Slab Thumbnails");
            for (int i = 0; i < static_cast<int>(slabs_.size()); ++i) {
                auto& slab = slabs_[i];
                if (!slab.valid()) continue;

                ImGui::BeginGroup();
                ImGui::Text("%s", slab.label().c_str());
                const ImVec2 thumbSize = fitSize(
                    static_cast<float>(slab.image().width()),
                    static_cast<float>(slab.image().height()),
                    itemW * 0.45f, itemH * 0.65f);
                ImGui::Image(static_cast<ImTextureID>(slab.image().texture()), thumbSize);

                ImVec2 depthPos = ImGui::GetCursorScreenPos();
                ImGui::InvisibleButton(("depth_paint_canvas_" + std::to_string(i)).c_str(),
                                       thumbSize,
                                       ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
                ImGui::GetWindowDrawList()->AddImage(static_cast<ImTextureID>(slab.depth().texture()),
                                                     depthPos,
                                                     ImVec2(depthPos.x + thumbSize.x, depthPos.y + thumbSize.y));

                const bool paintingActive = ImGui::IsItemHovered() || ImGui::IsItemActive();
                if (paintingActive && ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
                    paintSlabDepthFromUi(i, depthPos, thumbSize, false);
                }
                if (paintingActive && ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                    paintSlabDepthFromUi(i, depthPos, thumbSize, true);
                }
                ImGui::EndGroup();

                if (i < static_cast<int>(slabs_.size()) - 1) {
                    ImGui::SameLine();
                }
            }
        } else {
            ImGui::Text("Input Images");
            ImGui::Columns(2, "debug_views", false);

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
    }

    void paintDepthFromUi(ImVec2 imageMin, ImVec2 imageSize, bool smoothMode) {
        ImGuiIO& io = ImGui::GetIO();
        const float localX = io.MousePos.x - imageMin.x;
        const float localY = io.MousePos.y - imageMin.y;
        if (localX < 0.0f || localY < 0.0f || localX >= imageSize.x || localY >= imageSize.y) {
            return;
        }

        const float imgX = (localX / imageSize.x) * static_cast<float>(depth_.width());
        const float imgY = (localY / imageSize.y) * static_cast<float>(depth_.height());
        depth_.paintAt(imgX, imgY, settings_.brushRadius, settings_.brushDepth, settings_.brushStrength, smoothMode);
        markMeshDirtyIfInputsReady();
    }

    void paintSlabDepthFromUi(int slabIdx, ImVec2 imageMin, ImVec2 imageSize, bool smoothMode) {
        ImGuiIO& io = ImGui::GetIO();
        const float localX = io.MousePos.x - imageMin.x;
        const float localY = io.MousePos.y - imageMin.y;
        if (localX < 0.0f || localY < 0.0f || localX >= imageSize.x || localY >= imageSize.y) {
            return;
        }

        auto& depth = slabs_[slabIdx].depth();
        const float imgX = (localX / imageSize.x) * static_cast<float>(depth.width());
        const float imgY = (localY / imageSize.y) * static_cast<float>(depth.height());
        depth.paintAt(imgX, imgY, settings_.brushRadius, settings_.brushDepth, settings_.brushStrength, smoothMode);
        multiSlabMeshDirty_ = true;
    }

    DemoSettings settings_;
    ImageResource image_;
    DepthMap depth_;
    DepthMesh mesh_;
    WarpRenderer renderer_;
    std::string status_;
    bool meshDirty_ = true;
    bool exportResultRequested_ = false;

    std::vector<LightFieldSlab> slabs_;
    bool multiSlabMeshDirty_ = true;
};

}  // namespace

int runInteractiveApp(const std::string& inputPath) {
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

    app.shutdown();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
