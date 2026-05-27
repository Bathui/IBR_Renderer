#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <string>

class DepthMesh;
class ImageResource;

struct CameraSettings {
    float yaw = 18.0f;
    float pitch = -5.0f;
    float zoom = 2.25f;
    glm::vec2 pan = glm::vec2(0.0f);
    float fov = 38.0f;
    glm::vec3 bgColor = glm::vec3(0.94f, 0.94f, 0.92f);
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
    GLuint fbo() const { return fbo_; }
    int width() const { return width_; }
    int height() const { return height_; }

private:
    GLuint fbo_ = 0;
    GLuint color_ = 0;
    GLuint depth_ = 0;
    int width_ = 900;
    int height_ = 620;
};

class WarpRenderer {
public:
    bool init();
    void render(const ImageResource& image, const DepthMesh& mesh, const CameraSettings& camera);
    void shutdown();

    RenderTarget& target() { return target_; }
    const RenderTarget& target() const { return target_; }

private:
    ShaderProgram shader_;
    RenderTarget target_;
};
