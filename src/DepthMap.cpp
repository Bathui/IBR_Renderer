#include <DepthMap.h>

#include <ImageResource.h>
#include <ImageWrite.h>

#include <glm/glm.hpp>

#include <algorithm>
#include <cmath>

namespace {

// A tiny blur used for generated depth and the smoothing brush.
void smoothValues(std::vector<float>& values, int width, int height, int iterations) {
    if (iterations <= 0) {
        return;
    }

    std::vector<float> scratch(values.size());
    for (int it = 0; it < iterations; ++it) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float sum = 0.0f;
                float weight = 0.0f;
                for (int oy = -1; oy <= 1; ++oy) {
                    for (int ox = -1; ox <= 1; ++ox) {
                        const int sx = std::clamp(x + ox, 0, width - 1);
                        const int sy = std::clamp(y + oy, 0, height - 1);
                        const float w = (ox == 0 && oy == 0) ? 4.0f : 1.0f;
                        sum += values[sy * width + sx] * w;
                        weight += w;
                    }
                }
                scratch[y * width + x] = sum / weight;
            }
        }
        values.swap(scratch);
    }
}

}  // namespace

void DepthMap::generate(const ImageResource& image, float luminanceInfluence, int smoothingIterations, bool invertDepth) {
    // Allocate one floating-point depth value for each image pixel.
    width_ = image.width();
    height_ = image.height();
    values_.assign(width_ * height_, 0.0f);

    // This fallback is intentionally simple; loading a hand-made or estimated depth map is preferred.
    const float aspect = static_cast<float>(width_) / static_cast<float>(height_);
    for (int y = 0; y < height_; ++y) {
        const float ny = (static_cast<float>(y) / static_cast<float>(height_ - 1)) * 2.0f - 1.0f;
        for (int x = 0; x < width_; ++x) {
            const float nx = ((static_cast<float>(x) / static_cast<float>(width_ - 1)) * 2.0f - 1.0f) * aspect;
            const float faceBump = std::exp(-(nx * nx * 1.45f + ny * ny * 1.10f));
            const float vignette = std::exp(-(nx * nx * 0.35f + ny * ny * 0.45f));
            const float lum = image.luminanceAt(x, y);
            float d = 0.18f + 0.70f * faceBump + 0.12f * vignette;
            d = (1.0f - luminanceInfluence) * d + luminanceInfluence * lum;
            d = std::clamp(d, 0.0f, 1.0f);
            values_[y * width_ + x] = invertDepth ? 1.0f - d : d;
        }
    }

    smoothValues(values_, width_, height_, smoothingIterations);
    dirty_ = true;
}

void DepthMap::loadFromImage(const ImageResource& image, bool invertDepth) {
    // A loaded depth image becomes the authoritative depth map for the current scene.
    width_ = image.width();
    height_ = image.height();
    values_.assign(width_ * height_, 0.0f);

    // Any color image can be used as a depth map by reading its luminance.
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            float d = image.luminanceAt(x, y);
            values_[y * width_ + x] = invertDepth ? 1.0f - d : d;
        }
    }

    dirty_ = true;
}

void DepthMap::clear() {
    // Clear CPU data and release the preview texture together.
    width_ = 0;
    height_ = 0;
    values_.clear();
    previewRgba_.clear();
    dirty_ = false;
    destroyTexture();
}

void DepthMap::smoothCurrent(int iterations) {
    if (!valid()) {
        return;
    }
    smoothValues(values_, width_, height_, iterations);
    dirty_ = true;
}

void DepthMap::paintAt(float imgX, float imgY, float radius, float targetDepth, float strength, bool smoothMode) {
    if (!valid()) {
        return;
    }
    radius = std::max(1.0f, radius);
    const int minX = std::max(0, static_cast<int>(std::floor(imgX - radius)));
    const int maxX = std::min(width_ - 1, static_cast<int>(std::ceil(imgX + radius)));
    const int minY = std::max(0, static_cast<int>(std::floor(imgY - radius)));
    const int maxY = std::min(height_ - 1, static_cast<int>(std::ceil(imgY + radius)));

    // Smoothing reads from a copy so each pixel uses the same original neighborhood.
    std::vector<float> oldDepth;
    if (smoothMode) {
        oldDepth = values_;
    }

    // Circular brush with linear falloff from the center to the radius.
    for (int y = minY; y <= maxY; ++y) {
        for (int x = minX; x <= maxX; ++x) {
            const float dx = static_cast<float>(x) - imgX;
            const float dy = static_cast<float>(y) - imgY;
            const float dist = std::sqrt(dx * dx + dy * dy);
            if (dist > radius) {
                continue;
            }
            const float falloff = 1.0f - (dist / radius);
            const float alpha = std::clamp(strength * falloff, 0.0f, 1.0f);
            const int idx = y * width_ + x;
            if (smoothMode) {
                float sum = 0.0f;
                int count = 0;
                for (int oy = -2; oy <= 2; ++oy) {
                    for (int ox = -2; ox <= 2; ++ox) {
                        const int sx = std::clamp(x + ox, 0, width_ - 1);
                        const int sy = std::clamp(y + oy, 0, height_ - 1);
                        sum += oldDepth[sy * width_ + sx];
                        ++count;
                    }
                }
                values_[idx] = glm::mix(values_[idx], sum / static_cast<float>(count), alpha);
            } else {
                values_[idx] = glm::mix(values_[idx], targetDepth, alpha);
            }
        }
    }

    dirty_ = true;
}

float DepthMap::sample(float u, float v) const {
    if (!valid()) {
        return 0.0f;
    }
    // Bilinear filtering avoids blocky depth when the mesh is lower resolution than the image.
    const float x = std::clamp(u, 0.0f, 1.0f) * static_cast<float>(width_ - 1);
    const float y = std::clamp(v, 0.0f, 1.0f) * static_cast<float>(height_ - 1);
    const int x0 = static_cast<int>(std::floor(x));
    const int y0 = static_cast<int>(std::floor(y));
    const int x1 = std::min(x0 + 1, width_ - 1);
    const int y1 = std::min(y0 + 1, height_ - 1);
    const float tx = x - static_cast<float>(x0);
    const float ty = y - static_cast<float>(y0);
    const float a = values_[y0 * width_ + x0];
    const float b = values_[y0 * width_ + x1];
    const float c = values_[y1 * width_ + x0];
    const float d = values_[y1 * width_ + x1];
    return glm::mix(glm::mix(a, b, tx), glm::mix(c, d, tx), ty);
}

float DepthMap::sampleNearest(float u, float v) const {
    if (!valid()) {
        return 0.0f;
    }
    // Snap to the nearest integer texel — no blending at all.
    const int x = std::clamp(static_cast<int>(std::round(std::clamp(u, 0.0f, 1.0f) * static_cast<float>(width_ - 1))),
                             0, width_ - 1);
    const int y = std::clamp(static_cast<int>(std::round(std::clamp(v, 0.0f, 1.0f) * static_cast<float>(height_ - 1))),
                             0, height_ - 1);
    return values_[y * width_ + x];
}

void DepthMap::buildPreviewCpu() {
    if (!valid()) {
        previewRgba_.clear();
        return;
    }
    // Convert normalized float depth back to an RGBA grayscale texture for ImGui preview/export.
    previewRgba_.resize(width_ * height_ * 4);
    for (int i = 0; i < width_ * height_; ++i) {
        const auto v = static_cast<unsigned char>(std::clamp(values_[i], 0.0f, 1.0f) * 255.0f);
        previewRgba_[i * 4 + 0] = v;
        previewRgba_[i * 4 + 1] = v;
        previewRgba_[i * 4 + 2] = v;
        previewRgba_[i * 4 + 3] = 255;
    }
}

void DepthMap::updatePreviewTexture() {
    if (!valid()) {
        clear();
        return;
    }
    buildPreviewCpu();

    // Recreate the preview texture if a newly loaded depth map has a different size.
    const bool needsNewStorage = !previewTexture_ || previewTextureWidth_ != width_ || previewTextureHeight_ != height_;
    if (needsNewStorage) {
        if (previewTexture_) {
            glDeleteTextures(1, &previewTexture_);
        }
        glGenTextures(1, &previewTexture_);
        glBindTexture(GL_TEXTURE_2D, previewTexture_);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, previewRgba_.data());
        previewTextureWidth_ = width_;
        previewTextureHeight_ = height_;
    } else {
        glBindTexture(GL_TEXTURE_2D, previewTexture_);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, previewRgba_.data());
    }
    dirty_ = false;
}

bool DepthMap::savePreviewPng(const std::string& outputPath) {
    if (!valid()) {
        return false;
    }
    buildPreviewCpu();
    return writePngRgba(outputPath, width_, height_, previewRgba_);
}

void DepthMap::destroyTexture() {
    if (previewTexture_) {
        glDeleteTextures(1, &previewTexture_);
        previewTexture_ = 0;
    }
    previewTextureWidth_ = 0;
    previewTextureHeight_ = 0;
}
