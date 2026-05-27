#include <WarpRenderer.h>

#include <DepthMesh.h>
#include <ImageResource.h>
#include <ImageWrite.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <iostream>
#include <vector>

namespace {

// Minimal shader pair: transform the proxy mesh and texture it with the RGB image.
const char* kVertexSource = R"GLSL(
    #version 330 core
    layout (location = 0) in vec3 aPos;
    layout (location = 1) in vec2 aUv;
    uniform mat4 uMVP;
    out vec2 vUv;
    void main() {
        vUv = aUv;
        gl_Position = uMVP * vec4(aPos, 1.0);
    }
)GLSL";

const char* kFragmentSource = R"GLSL(
    #version 330 core
    in vec2 vUv;
    uniform sampler2D uImage;
    out vec4 fragColor;
    void main() {
        fragColor = texture(uImage, vUv);
    }
)GLSL";

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[2048] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::cerr << "Shader compile failed:\n" << log << "\n";
    }
    return shader;
}

}  // namespace

bool ShaderProgram::create(const char* vertexSource, const char* fragmentSource) {
    // Compile both shader stages and link them into a reusable program.
    GLuint vs = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    id_ = glCreateProgram();
    glAttachShader(id_, vs);
    glAttachShader(id_, fs);
    glLinkProgram(id_);

    GLint ok = GL_FALSE;
    glGetProgramiv(id_, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[2048] = {};
        glGetProgramInfoLog(id_, sizeof(log), nullptr, log);
        std::cerr << "Program link failed:\n" << log << "\n";
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return ok == GL_TRUE;
}

void ShaderProgram::destroy() {
    if (id_) {
        glDeleteProgram(id_);
        id_ = 0;
    }
}

void RenderTarget::ensure(int width, int height) {
    // Recreate framebuffer storage only when the requested size changes.
    width = std::max(16, width);
    height = std::max(16, height);
    if (fbo_ && width_ == width && height_ == height) {
        return;
    }

    width_ = width;
    height_ = height;
    if (!fbo_) {
        glGenFramebuffers(1, &fbo_);
        glGenTextures(1, &color_);
        glGenRenderbuffers(1, &depth_);
    }

    glBindTexture(GL_TEXTURE_2D, color_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindRenderbuffer(GL_RENDERBUFFER, depth_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_, height_);

    // The color texture is what gets shown on screen and exported in screenshots.
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, depth_);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Render target framebuffer is incomplete.\n";
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool RenderTarget::savePng(const std::string& outputPath) const {
    std::vector<unsigned char> pixels(width_ * height_ * 4);
    std::vector<unsigned char> flipped(width_ * height_ * 4);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // OpenGL reads pixels from bottom to top; PNG files expect top to bottom.
    const int rowBytes = width_ * 4;
    for (int y = 0; y < height_; ++y) {
        const int srcY = height_ - 1 - y;
        std::copy_n(pixels.data() + srcY * rowBytes, rowBytes, flipped.data() + y * rowBytes);
    }

    return writePngRgba(outputPath, width_, height_, flipped);
}

void RenderTarget::blitToDefaultFramebuffer(int framebufferWidth, int framebufferHeight) const {
    const float srcAspect = static_cast<float>(width_) / static_cast<float>(height_);
    const float dstAspect = static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight);

    int drawW = framebufferWidth;
    int drawH = framebufferHeight;
    if (dstAspect > srcAspect) {
        drawW = static_cast<int>(static_cast<float>(framebufferHeight) * srcAspect);
    } else {
        drawH = static_cast<int>(static_cast<float>(framebufferWidth) / srcAspect);
    }

    const int x0 = (framebufferWidth - drawW) / 2;
    const int y0 = (framebufferHeight - drawH) / 2;
    const int x1 = x0 + drawW;
    const int y1 = y0 + drawH;

    // Preserve aspect ratio when copying the offscreen result to the window.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width_, height_, x0, y0, x1, y1, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget::destroy() {
    if (depth_) glDeleteRenderbuffers(1, &depth_);
    if (color_) glDeleteTextures(1, &color_);
    if (fbo_) glDeleteFramebuffers(1, &fbo_);
    depth_ = 0;
    color_ = 0;
    fbo_ = 0;
}

bool WarpRenderer::init() {
    target_.ensure(900, 620);
    return shader_.create(kVertexSource, kFragmentSource);
}

void WarpRenderer::render(const ImageResource& image, const DepthMesh& mesh, const CameraSettings& camera) {
    // Render the warped mesh offscreen first. The main window copies this result afterward.
    glBindFramebuffer(GL_FRAMEBUFFER, target_.fbo());
    glViewport(0, 0, target_.width(), target_.height());
    glEnable(GL_DEPTH_TEST);
    glClearColor(camera.bgColor.r, camera.bgColor.g, camera.bgColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Build camera matrices from UI sliders.
    const float aspect = static_cast<float>(target_.width()) / static_cast<float>(target_.height());
    const glm::mat4 projection = glm::perspective(glm::radians(camera.fov), aspect, 0.05f, 20.0f);
    const glm::mat4 view = glm::lookAt(glm::vec3(camera.pan.x, camera.pan.y, camera.zoom),
                                      glm::vec3(camera.pan.x, camera.pan.y, 0.0f),
                                      glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model(1.0f);
    model = glm::rotate(model, glm::radians(camera.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(camera.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 mvp = projection * view * model;

    // The shader is simple: transform the displaced mesh and sample the original RGB texture.
    glUseProgram(shader_.id());
    glUniformMatrix4fv(glGetUniformLocation(shader_.id(), "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform1i(glGetUniformLocation(shader_.id(), "uImage"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image.texture());
    glBindVertexArray(mesh.vao());

    glPolygonMode(GL_FRONT_AND_BACK, camera.wireframe ? GL_LINE : GL_FILL);
    glDrawElements(GL_TRIANGLES, mesh.indexCount(), GL_UNSIGNED_INT, nullptr);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void WarpRenderer::shutdown() {
    target_.destroy();
    shader_.destroy();
}
