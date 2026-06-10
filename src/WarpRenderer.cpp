#include <WarpRenderer.h>

#include <DepthMesh.h>
#include <ImageResource.h>
#include <ImageWrite.h>
#include <LightFieldSlab.h>

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
        // Output alpha=1 to mark this pixel as foreground geometry.
        fragColor = vec4(texture(uImage, vUv).rgb, 1.0);
    }
)GLSL";

const char* kVQFragmentSource = R"GLSL(
    #version 330 core
    in vec2 vUv;
    uniform usampler2D uIndexMap;
    uniform sampler1D uCodebook;
    out vec4 fragColor;
    void main() {
        ivec2 indexSize = textureSize(uIndexMap, 0);
        vec2 p = vUv * vec2(indexSize) * 2.0;
        
        ivec2 p_int = ivec2(clamp(p, vec2(0.0), vec2(indexSize * 2) - 1.0));
        ivec2 b = p_int / 2;
        ivec2 offset = p_int % 2;
        
        uint k = texelFetch(uIndexMap, b, 0).r;
        int codebookIdx = int(k * 4u) + offset.y * 2 + offset.x;
        
        fragColor = vec4(texelFetch(uCodebook, codebookIdx, 0).rgb, 1.0);
    }
)GLSL";

// Fullscreen-quad shader for blending two render targets together.
const char* kBlendVertexSource = R"GLSL(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aUv;
    out vec2 vUv;
    void main() {
        vUv = aUv;
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
)GLSL";

const char* kBlendFragmentSource = R"GLSL(
    #version 330 core
    in vec2 vUv;
    uniform sampler2D uSlabA;
    uniform sampler2D uSlabB;
    uniform sampler2D uDepthA;
    uniform sampler2D uDepthB;
    uniform float uBlendWeight;
    out vec4 fragColor;
    void main() {
        vec4 a = texture(uSlabA, vUv);
        vec4 b = texture(uSlabB, vUv);
        float dA = texture(uDepthA, vUv).r;
        float dB = texture(uDepthB, vUv).r;
        
        // Use the alpha channel to only blend valid foreground pixels.
        // This prevents the background color from bleeding into the object.
        if (a.a > 0.0 && b.a > 0.0) {
            float depthDiff = dB - dA;
            float bias = 0.01; // Depth tolerance
            if (depthDiff > bias) {
                fragColor = a; // A is much closer, occludes B
            } else if (depthDiff < -bias) {
                fragColor = b; // B is much closer, occludes A
            } else {
                fragColor = vec4(mix(a.rgb, b.rgb, uBlendWeight), 1.0);
            }
        } else if (a.a > 0.0) {
            fragColor = a;
        } else if (b.a > 0.0) {
            fragColor = b;
        } else {
            fragColor = vec4(a.rgb, 0.0); // Both background
        }
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

    // The color texture is what gets shown on screen and exported in screenshots.
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
    if (depthTexture_) glDeleteTextures(1, &depthTexture_);
    if (color_) glDeleteTextures(1, &color_);
    if (fbo_) glDeleteFramebuffers(1, &fbo_);
    depthTexture_ = 0;
    color_ = 0;
    fbo_ = 0;
}

bool WarpRenderer::init() {
    target_.ensure(900, 620);

    if (!shader_.create(kVertexSource, kFragmentSource)) {
        return false;
    }
    if (!blendShader_.create(kBlendVertexSource, kBlendFragmentSource)) {
        return false;
    }
    if (!vqShader_.create(kVertexSource, kVQFragmentSource)) {
        return false;
    }

    // Create a fullscreen quad for the blend pass.
    // Two triangles covering NDC [-1,1] x [-1,1].
    float quadVerts[] = {
        // pos.x  pos.y  uv.x  uv.y
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
    // Render the warped mesh offscreen first. The main window copies this result afterward.
    glBindFramebuffer(GL_FRAMEBUFFER, target_.fbo());
    glViewport(0, 0, target_.width(), target_.height());
    glEnable(GL_DEPTH_TEST);
    // Ensure final background is opaque if nothing is rendered
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
    // Clear with alpha 0 to distinguish background from foreground geometry.
    glClearColor(camera.bgColor.r, camera.bgColor.g, camera.bgColor.b, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const float aspect = static_cast<float>(dst.width()) / static_cast<float>(dst.height());
    const glm::mat4 projection = glm::perspective(glm::radians(camera.fov), aspect, 0.05f, 20.0f);
    const glm::mat4 view = glm::lookAt(glm::vec3(camera.pan.x, camera.pan.y, camera.zoom),
                                       glm::vec3(camera.pan.x, camera.pan.y, 0.0f),
                                       glm::vec3(0.0f, 1.0f, 0.0f));

    // The mesh should only warp by the DELTA between the current camera angle and
    // the slab's original capture direction.  Each slab was captured head-on from
    // its orientation, so a small delta produces the correct depth-based parallax.
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
        // Quadrilinear: render both closest slabs, then blend.
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
        // Nearest or fallback: render only the closest slab.
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
