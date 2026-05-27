#pragma once

#include <glad/glad.h>

#include <string>
#include <vector>

class ImageResource {
public:
    // CPU-only loading is useful for depth maps; texture loading is used for RGB images.
    bool loadCpuOnly(const std::string& path);
    bool loadWithTexture(const std::string& path);
    void destroyTexture();

    int width() const { return width_; }
    int height() const { return height_; }
    GLuint texture() const { return texture_; }
    bool valid() const { return width_ > 0 && height_ > 0 && !rgba_.empty(); }
    const std::vector<unsigned char>& rgba() const { return rgba_; }

    // Converts the RGB pixel at (x, y) into a single grayscale value in [0, 1].
    float luminanceAt(int x, int y) const;

private:
    bool loadPixels(const std::string& path);
    void uploadTexture();

    int width_ = 0;
    int height_ = 0;
    int channels_ = 0;
    std::vector<unsigned char> rgba_;
    GLuint texture_ = 0;
};
