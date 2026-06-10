#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

class DepthMap;
class ImageResource;

class DepthMesh {
public:
    void rebuild(const ImageResource& image,
                 const DepthMap& depth,
                 int cols,
                 int rows,
                 float depthScale,
                 float depthBias,
                 float tearThreshold,
                 float backgroundCutoff = 0.0f);
    void destroy();

    GLuint vao() const { return vao_; }
    GLsizei indexCount() const { return indexCount_; }

private:
    struct Vertex {
        glm::vec3 position;
        glm::vec2 uv;
    };

    GLuint vao_ = 0;
    GLuint vbo_ = 0;
    GLuint ebo_ = 0;
    GLsizei indexCount_ = 0;
};
