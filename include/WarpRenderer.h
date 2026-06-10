#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>

class DepthMesh;
class ImageResource;
class LightFieldSlab;
enum class InterpolationMode;
struct SlabOrientation;

struct CameraSettings {
    float yaw = 18.0f;
    float pitch = -5.0f;
    float zoom = 2.25f;
    glm::vec2 pan = glm::vec2(0.0f);
    float fov = 38.0f;
    glm::vec3 bgColor = glm::vec3(0.231f, 0.231f, 0.231f);
    bool wireframe = false;
};

class ShaderProgram {
public:
    bool create(const char* vertexSource, const char* fragmentSource);
    void destroy();
    GLuint id() const { return id_; }

private:
    GLuint id_ = 0;
};

class RenderTarget {
public:
    // Offscreen rendering keeps screenshots and on-screen drawing using the same result.
    void ensure(int width, int height);
    bool savePng(const std::string& outputPath) const;
    void blitToDefaultFramebuffer(int framebufferWidth, int framebufferHeight) const;
    void destroy();

    GLuint colorTexture() const { return color_; }
    GLuint depthTexture() const { return depthTexture_; }
    GLuint fbo() const { return fbo_; }
    int width() const { return width_; }
    int height() const { return height_; }

private:
    GLuint fbo_ = 0;
    GLuint color_ = 0;
    GLuint depthTexture_ = 0;
    int width_ = 900;
    int height_ = 620;
};

class WarpRenderer {
public:
    bool init();
    // Original single-slab render path (kept for backward compatibility).
    void render(const ImageResource& image, const DepthMesh& mesh, const CameraSettings& camera);

    // Multi-slab render path: renders one or two slabs and composites.
    void renderMultiSlab(const std::vector<LightFieldSlab>& slabs,
                         const CameraSettings& camera,
                         InterpolationMode mode);

    void shutdown();

    RenderTarget& target() { return target_; }
    const RenderTarget& target() const { return target_; }

private:
    // Render a single slab's mesh with a model-space rotation for its orientation.
    void renderSlabToTarget(const ImageResource& image, const DepthMesh& mesh,
                            const CameraSettings& camera, const SlabOrientation& orient,
                            RenderTarget& dst, GLint filterMode = GL_LINEAR);

    // Fullscreen-quad blend of two render targets.
    void blendTargets(const RenderTarget& a, const RenderTarget& b,
                      float weight, RenderTarget& dst);

    ShaderProgram shader_;         // textured-mesh shader
    ShaderProgram vqShader_;       // VQ-decompression shader
    ShaderProgram blendShader_;    // fullscreen-quad blend shader
    GLuint        blendVao_ = 0;
    GLuint        blendVbo_ = 0;
    RenderTarget  target_;         // primary / final output
    RenderTarget  targetB_;        // secondary slab for quadrilinear blending
};
