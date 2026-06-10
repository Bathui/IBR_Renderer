#include <WarpRenderer.h>

#include <DepthMesh.h>
#include <ImageResource.h>
#include <ImageWrite.h>
#include <LightFieldSlab.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

namespace {

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << "\n";
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

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
        glGenTextures(1, &depthTexture_);
    }

    glBindTexture(GL_TEXTURE_2D, color_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

    glBindTexture(GL_TEXTURE_2D, depthTexture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width_, height_, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, nullptr);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depthTexture_, 0);
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

    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, width_, height_, x0, y0, x1, y1, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void RenderTarget::destroy() {
    if (depthTexture_) glDeleteTextures(1, &depthTexture_);
    if (color_) glDeleteTextures(1, &color_);
    if (fbo_) glDeleteFramebuffers(1, &fbo_);
    depthTexture_ = 0;
    color_ = 0;
    fbo_ = 0;
}

bool WarpRenderer::init() {
    target_.ensure(900, 620);

    std::string vs = readFile("Shaders/main.vert");
    std::string fs = readFile("Shaders/main.frag");
    std::string vqFs = readFile("Shaders/vq.frag");
    std::string blendVs = readFile("Shaders/blend.vert");
    std::string blendFs = readFile("Shaders/blend.frag");

    if (vs.empty() || fs.empty() || vqFs.empty() || blendVs.empty() || blendFs.empty()) {
        std::cerr << "Failed to load one or more shaders.\n";
        return false;
    }

    if (!shader_.create(vs.c_str(), fs.c_str())) {
        return false;
    }
    if (!blendShader_.create(blendVs.c_str(), blendFs.c_str())) {
        return false;
    }
    if (!vqShader_.create(vs.c_str(), vqFs.c_str())) {
        return false;
    }

    float quadVerts[] = {
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,

        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
    };

    glGenVertexArrays(1, &blendVao_);
    glGenBuffers(1, &blendVbo_);
    glBindVertexArray(blendVao_);
    glBindBuffer(GL_ARRAY_BUFFER, blendVbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    return true;
}

void WarpRenderer::render(const ImageResource& image, const DepthMesh& mesh, const CameraSettings& camera) {
    glBindFramebuffer(GL_FRAMEBUFFER, target_.fbo());
    glViewport(0, 0, target_.width(), target_.height());
    glEnable(GL_DEPTH_TEST);
    glClearColor(camera.bgColor.r, camera.bgColor.g, camera.bgColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float aspect = static_cast<float>(target_.width()) / static_cast<float>(target_.height());
    const glm::mat4 projection = glm::perspective(glm::radians(camera.fov), aspect, 0.05f, 20.0f);
    const glm::mat4 view = glm::lookAt(glm::vec3(camera.pan.x, camera.pan.y, camera.zoom),
                                      glm::vec3(camera.pan.x, camera.pan.y, 0.0f),
                                      glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 model(1.0f);
    model = glm::rotate(model, glm::radians(camera.pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(camera.yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 mvp = projection * view * model;

    if (image.isVQ()) {
        glUseProgram(vqShader_.id());
        glUniformMatrix4fv(glGetUniformLocation(vqShader_.id(), "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform1i(glGetUniformLocation(vqShader_.id(), "uIndexMap"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, image.indexTexture());
        glUniform1i(glGetUniformLocation(vqShader_.id(), "uCodebook"), 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, image.codebookTexture());
    } else {
        glUseProgram(shader_.id());
        glUniformMatrix4fv(glGetUniformLocation(shader_.id(), "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform1i(glGetUniformLocation(shader_.id(), "uImage"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, image.texture());
    }
    glBindVertexArray(mesh.vao());

    glPolygonMode(GL_FRONT_AND_BACK, camera.wireframe ? GL_LINE : GL_FILL);
    glDrawElements(GL_TRIANGLES, mesh.indexCount(), GL_UNSIGNED_INT, nullptr);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void WarpRenderer::renderSlabToTarget(const ImageResource& image, const DepthMesh& mesh,
                                       const CameraSettings& camera, const SlabOrientation& orient,
                                       RenderTarget& dst, GLint filterMode) {
    dst.ensure(target_.width(), target_.height());

    glBindFramebuffer(GL_FRAMEBUFFER, dst.fbo());
    glViewport(0, 0, dst.width(), dst.height());
    glEnable(GL_DEPTH_TEST);
    glClearColor(camera.bgColor.r, camera.bgColor.g, camera.bgColor.b, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float aspect = static_cast<float>(dst.width()) / static_cast<float>(dst.height());
    const glm::mat4 projection = glm::perspective(glm::radians(camera.fov), aspect, 0.05f, 20.0f);
    const glm::mat4 view = glm::lookAt(glm::vec3(camera.pan.x, camera.pan.y, camera.zoom),
                                       glm::vec3(camera.pan.x, camera.pan.y, 0.0f),
                                       glm::vec3(0.0f, 1.0f, 0.0f));

    const float deltaYaw   = camera.yaw   - orient.yaw;
    const float deltaPitch = camera.pitch  - orient.pitch;
    glm::mat4 model(1.0f);
    model = glm::rotate(model, glm::radians(deltaPitch), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, glm::radians(deltaYaw),   glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 mvp = projection * view * model;

    if (image.isVQ()) {
        glUseProgram(vqShader_.id());
        glUniformMatrix4fv(glGetUniformLocation(vqShader_.id(), "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform1i(glGetUniformLocation(vqShader_.id(), "uIndexMap"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, image.indexTexture());
        glUniform1i(glGetUniformLocation(vqShader_.id(), "uCodebook"), 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_1D, image.codebookTexture());
    } else {
        glUseProgram(shader_.id());
        glUniformMatrix4fv(glGetUniformLocation(shader_.id(), "uMVP"), 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform1i(glGetUniformLocation(shader_.id(), "uImage"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, image.texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filterMode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filterMode);
    }
    glBindVertexArray(mesh.vao());

    glPolygonMode(GL_FRONT_AND_BACK, camera.wireframe ? GL_LINE : GL_FILL);
    glDrawElements(GL_TRIANGLES, mesh.indexCount(), GL_UNSIGNED_INT, nullptr);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void WarpRenderer::blendTargets(const RenderTarget& a, const RenderTarget& b,
                                 float weight, RenderTarget& dst) {
    dst.ensure(a.width(), a.height());

    glBindFramebuffer(GL_FRAMEBUFFER, dst.fbo());
    glViewport(0, 0, dst.width(), dst.height());
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(blendShader_.id());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, a.colorTexture());
    glUniform1i(glGetUniformLocation(blendShader_.id(), "uSlabA"), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, b.colorTexture());
    glUniform1i(glGetUniformLocation(blendShader_.id(), "uSlabB"), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, a.depthTexture());
    glUniform1i(glGetUniformLocation(blendShader_.id(), "uDepthA"), 2);

    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, b.depthTexture());
    glUniform1i(glGetUniformLocation(blendShader_.id(), "uDepthB"), 3);

    glUniform1f(glGetUniformLocation(blendShader_.id(), "uBlendWeight"), weight);

    glBindVertexArray(blendVao_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void WarpRenderer::renderMultiSlab(const std::vector<LightFieldSlab>& slabs,
                                    const CameraSettings& camera,
                                    InterpolationMode mode) {
    if (slabs.empty()) return;

    GLint filterMode = (mode == InterpolationMode::Nearest) ? GL_NEAREST : GL_LINEAR;

    SlabSelection sel = selectClosestSlabs(slabs, camera.yaw, camera.pitch);

    if (sel.primaryIdx < 0) {
        return;
    }

    const auto& primary = slabs[sel.primaryIdx];
    if (!primary.meshReady()) {
        return;
    }

    if (mode == InterpolationMode::Quadrilinear && sel.secondaryIdx >= 0 &&
        slabs[sel.secondaryIdx].meshReady() && sel.blendWeight > 0.001f) {
        RenderTarget tempA;
        tempA.ensure(target_.width(), target_.height());

        renderSlabToTarget(primary.image(), primary.mesh(), camera,
                           primary.orientation(), tempA, filterMode);

        const auto& secondary = slabs[sel.secondaryIdx];
        targetB_.ensure(target_.width(), target_.height());
        renderSlabToTarget(secondary.image(), secondary.mesh(), camera,
                           secondary.orientation(), targetB_, filterMode);

        blendTargets(tempA, targetB_, sel.blendWeight, target_);
        tempA.destroy();
    } else {
        renderSlabToTarget(primary.image(), primary.mesh(), camera,
                           primary.orientation(), target_, filterMode);
    }
}

void WarpRenderer::shutdown() {
    target_.destroy();
    targetB_.destroy();
    shader_.destroy();
    vqShader_.destroy();
    blendShader_.destroy();
    if (blendVbo_) glDeleteBuffers(1, &blendVbo_);
    if (blendVao_) glDeleteVertexArrays(1, &blendVao_);
    blendVbo_ = 0;
    blendVao_ = 0;
}
