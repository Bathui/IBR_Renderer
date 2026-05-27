#include <DepthMesh.h>

#include <DepthMap.h>
#include <ImageResource.h>

#include <algorithm>
#include <vector>

namespace {

bool triangleIsContinuous(float a, float b, float c, float threshold) {
    // A high threshold effectively disables tearing.
    if (threshold >= 1.0f) {
        return true;
    }

    const float minDepth = std::min({a, b, c});
    const float maxDepth = std::max({a, b, c});
    return (maxDepth - minDepth) <= threshold;
}

}  // namespace

void DepthMesh::rebuild(const ImageResource& image,
                        const DepthMap& depth,
                        int cols,
                        int rows,
                        float depthScale,
                        float depthBias,
                        float tearThreshold) {
    // Keep the mesh in the same aspect ratio as the source image.
    const float aspect = static_cast<float>(image.width()) / static_cast<float>(image.height());
    cols = std::max(8, cols);
    rows = std::max(8, rows);

    std::vector<Vertex> vertices;
    std::vector<float> vertexDepths;
    vertices.reserve(cols * rows);
    vertexDepths.reserve(cols * rows);

    // Make a regular image-plane grid and push each vertex along z using depth.
    for (int y = 0; y < rows; ++y) {
        const float v = static_cast<float>(y) / static_cast<float>(rows - 1);
        for (int x = 0; x < cols; ++x) {
            const float u = static_cast<float>(x) / static_cast<float>(cols - 1);
            const float d = depth.sample(u, v);
            const float px = (u - 0.5f) * aspect;
            const float py = 0.5f - v;
            const float pz = d * depthScale + depthBias;
            vertices.push_back({glm::vec3(px, py, pz), glm::vec2(u, v)});
            vertexDepths.push_back(d);
        }
    }

    std::vector<unsigned int> indices;
    indices.reserve((cols - 1) * (rows - 1) * 6);

    // Skip triangles across sharp depth jumps. This leaves holes instead of long texture streaks.
    for (int y = 0; y < rows - 1; ++y) {
        for (int x = 0; x < cols - 1; ++x) {
            const unsigned int i0 = static_cast<unsigned int>(y * cols + x);
            const unsigned int i1 = i0 + 1;
            const unsigned int i2 = static_cast<unsigned int>((y + 1) * cols + x);
            const unsigned int i3 = i2 + 1;
            if (triangleIsContinuous(vertexDepths[i0], vertexDepths[i2], vertexDepths[i1], tearThreshold)) {
                indices.push_back(i0);
                indices.push_back(i2);
                indices.push_back(i1);
            }
            if (triangleIsContinuous(vertexDepths[i1], vertexDepths[i2], vertexDepths[i3], tearThreshold)) {
                indices.push_back(i1);
                indices.push_back(i2);
                indices.push_back(i3);
            }
        }
    }

    // Allocate the OpenGL buffers on the first rebuild; later rebuilds reuse them.
    if (!vao_) {
        glGenVertexArrays(1, &vao_);
        glGenBuffers(1, &vbo_);
        glGenBuffers(1, &ebo_);
    }

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)), vertices.data(), GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)), indices.data(), GL_STATIC_DRAW);

    // Attribute 0 is position; attribute 1 is the UV used to sample the RGB texture.
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, position)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, uv)));
    glBindVertexArray(0);

    indexCount_ = static_cast<GLsizei>(indices.size());
}

void DepthMesh::destroy() {
    if (ebo_) glDeleteBuffers(1, &ebo_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    ebo_ = 0;
    vbo_ = 0;
    vao_ = 0;
    indexCount_ = 0;
}
