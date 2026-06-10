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
    if (indexTexture_) {
        glDeleteTextures(1, &indexTexture_);
        indexTexture_ = 0;
    }
    if (codebookTexture_) {
        glDeleteTextures(1, &codebookTexture_);
        codebookTexture_ = 0;
    }
}

bool ImageResource::compressVQ() {
    if (rgba_.empty()) return false;

    const int numBlocksX = width_ / 2;
    const int numBlocksY = height_ / 2;
    const int numBlocks = numBlocksX * numBlocksY;
    const int K = 256;

    if (numBlocks == 0) return false;

    std::vector<std::vector<float>> blocks(numBlocks, std::vector<float>(16));
    for (int by = 0; by < numBlocksY; ++by) {
        for (int bx = 0; bx < numBlocksX; ++bx) {
            int blockIdx = by * numBlocksX + bx;
            for (int dy = 0; dy < 2; ++dy) {
                for (int dx = 0; dx < 2; ++dx) {
                    int px = bx * 2 + dx;
                    int py = by * 2 + dy;
                    int pIdx = (py * width_ + px) * 4;
                    int outIdx = (dy * 2 + dx) * 4;
                    blocks[blockIdx][outIdx + 0] = rgba_[pIdx + 0] / 255.0f;
                    blocks[blockIdx][outIdx + 1] = rgba_[pIdx + 1] / 255.0f;
                    blocks[blockIdx][outIdx + 2] = rgba_[pIdx + 2] / 255.0f;
                    blocks[blockIdx][outIdx + 3] = rgba_[pIdx + 3] / 255.0f;
                }
            }
        }
    }

    std::vector<std::vector<float>> codebook(K, std::vector<float>(16));
    for (int k = 0; k < K; ++k) {
        codebook[k] = blocks[(k * 97) % numBlocks];
    }

    indices_.assign(numBlocks, 0);
    std::vector<int> counts(K);

    for (int iter = 0; iter < 5; ++iter) {
        // Assign blocks to nearest codebook entry
        for (int b = 0; b < numBlocks; ++b) {
            float bestDist = 1e9f;
            int bestK = 0;
            for (int k = 0; k < K; ++k) {
                float dist = 0.0f;
                for (int i = 0; i < 16; ++i) {
                    float d = blocks[b][i] - codebook[k][i];
                    dist += d * d;
                }
                if (dist < bestDist) {
                    bestDist = dist;
                    bestK = k;
                }
            }
            indices_[b] = static_cast<unsigned char>(bestK);
        }

        // Update codebook
        std::vector<std::vector<float>> newCodebook(K, std::vector<float>(16, 0.0f));
        std::fill(counts.begin(), counts.end(), 0);

        for (int b = 0; b < numBlocks; ++b) {
            int k = indices_[b];
            counts[k]++;
            for (int i = 0; i < 16; ++i) {
                newCodebook[k][i] += blocks[b][i];
            }
        }

        for (int k = 0; k < K; ++k) {
            if (counts[k] > 0) {
                for (int i = 0; i < 16; ++i) {
                    codebook[k][i] = newCodebook[k][i] / counts[k];
                }
            }
        }
    }

    codebook_.resize(K * 16);
    for (int k = 0; k < K; ++k) {
        for (int i = 0; i < 16; ++i) {
            codebook_[k * 16 + i] = static_cast<unsigned char>(std::clamp(codebook[k][i] * 255.0f, 0.0f, 255.0f));
        }
    }

    isVQ_ = true;
    return true;
}

void ImageResource::uploadTextureVQ() {
    if (indexTexture_) glDeleteTextures(1, &indexTexture_);
    if (codebookTexture_) glDeleteTextures(1, &codebookTexture_);

    glGenTextures(1, &indexTexture_);
    glBindTexture(GL_TEXTURE_2D, indexTexture_);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8UI, width_ / 2, height_ / 2, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE, indices_.data());

    glGenTextures(1, &codebookTexture_);
    glBindTexture(GL_TEXTURE_1D, codebookTexture_);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 256 * 4, 0, GL_RGBA, GL_UNSIGNED_BYTE, codebook_.data());
}

void ImageResource::disableVQ() {
    isVQ_ = false;
    if (indexTexture_) {
        glDeleteTextures(1, &indexTexture_);
        indexTexture_ = 0;
    }
    if (codebookTexture_) {
        glDeleteTextures(1, &codebookTexture_);
        codebookTexture_ = 0;
    }
    // Restore regular texture
    uploadTexture();
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
