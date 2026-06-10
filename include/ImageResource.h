#pragma once

#include <glad/glad.h>

#include <string>
#include <vector>

class ImageResource {
public:
    bool loadCpuOnly(const std::string& path);
    bool loadWithTexture(const std::string& path);
    void destroyTexture();

    bool compressVQ();
    void uploadTextureVQ();
    void disableVQ();

    bool isVQ() const { return isVQ_; }
    GLuint indexTexture() const { return indexTexture_; }
    GLuint codebookTexture() const { return codebookTexture_; }

    int width() const { return width_; }
    int height() const { return height_; }
    GLuint texture() const { return texture_; }
    bool valid() const { return width_ > 0 && height_ > 0 && !rgba_.empty(); }
    const std::vector<unsigned char>& rgba() const { return rgba_; }

    float luminanceAt(int x, int y) const;

private:
    bool loadPixels(const std::string& path);
    void uploadTexture();

    int width_ = 0;
    int height_ = 0;
    int channels_ = 0;
    std::vector<unsigned char> rgba_;
    GLuint texture_ = 0;

    bool isVQ_ = false;
    GLuint indexTexture_ = 0;
    GLuint codebookTexture_ = 0;
    std::vector<unsigned char> indices_;
    std::vector<unsigned char> codebook_;
};
