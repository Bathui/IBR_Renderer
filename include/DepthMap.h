#pragma once

#include <glad/glad.h>

#include <string>
#include <vector>

class ImageResource;

// Stores the grayscale depth data used to displace the image mesh.
// Depth values are normalized: 0 is far, 1 is near.
class DepthMap {
public:
    void generate(const ImageResource& image, float luminanceInfluence, int smoothingIterations, bool invertDepth);
    void loadFromImage(const ImageResource& image, bool invertDepth);
    void clear();
    void smoothCurrent(int iterations);
    void paintAt(float imgX, float imgY, float radius, float targetDepth, float strength, bool smoothMode);

    void updatePreviewTexture();
    bool savePreviewPng(const std::string& outputPath);
    void destroyTexture();

    // Bilinear lookup so the mesh resolution can differ from the depth image resolution.
    float sample(float u, float v) const;
    int width() const { return width_; }
    int height() const { return height_; }
    GLuint texture() const { return previewTexture_; }
    bool valid() const { return width_ > 0 && height_ > 0 && !values_.empty(); }
    bool matchesSize(int width, int height) const { return valid() && width_ == width && height_ == height; }
    bool dirty() const { return dirty_; }
    void markDirty() { dirty_ = true; }

private:
    void buildPreviewCpu();

    int width_ = 0;
    int height_ = 0;
    std::vector<float> values_;
    std::vector<unsigned char> previewRgba_;
    GLuint previewTexture_ = 0;
    int previewTextureWidth_ = 0;
    int previewTextureHeight_ = 0;
    bool dirty_ = true;
};
