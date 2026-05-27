#include <ImageResource.h>

#include <algorithm>
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

bool ImageResource::loadCpuOnly(const std::string& path) {
    return loadPixels(path);
}

bool ImageResource::loadWithTexture(const std::string& path) {
    // RGB images need both CPU pixels and a GPU texture for rendering.
    if (!loadPixels(path)) {
        return false;
    }
    uploadTexture();
    return true;
}

bool ImageResource::loadPixels(const std::string& path) {
    // stb_image normalizes all formats to 4 channels so the rest of the code can assume RGBA.
    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc* pixels = stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!pixels) {
        std::cerr << "Failed to load image: " << path << "\n";
        return false;
    }

    width_ = width;
    height_ = height;
    channels_ = 4;
    rgba_.assign(pixels, pixels + width * height * 4);
    stbi_image_free(pixels);
    return true;
}

void ImageResource::uploadTexture() {
    // Replace the old texture if the user loads a different image.
    if (texture_) {
        glDeleteTextures(1, &texture_);
    }

    // Clamp-to-edge avoids sampling wrapped pixels along the border of the image mesh.
    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width_, height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba_.data());
}

void ImageResource::destroyTexture() {
    if (texture_) {
        glDeleteTextures(1, &texture_);
        texture_ = 0;
    }
}

float ImageResource::luminanceAt(int x, int y) const {
    // Standard perceptual RGB weights make depth images work whether they are RGB or grayscale.
    x = std::clamp(x, 0, width_ - 1);
    y = std::clamp(y, 0, height_ - 1);
    const int idx = (y * width_ + x) * 4;
    const float r = rgba_[idx + 0] / 255.0f;
    const float g = rgba_[idx + 1] / 255.0f;
    const float b = rgba_[idx + 2] / 255.0f;
    return 0.2126f * r + 0.7152f * g + 0.0722f * b;
}
