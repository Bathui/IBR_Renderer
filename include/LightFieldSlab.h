#pragma once

#include <DepthMap.h>
#include <DepthMesh.h>
#include <ImageResource.h>

#include <string>
#include <vector>

// Interpolation mode for depth sampling and cross-slab blending.
enum class InterpolationMode {
    Nearest,       // snap to closest texel; render only the single nearest slab
    Bilinear,      // 2x2 linear depth interpolation; render the nearest slab
    Quadrilinear   // bilinear per slab + weighted blend of the two closest slabs
};

// Describes how a slab is oriented relative to the object center.
struct SlabOrientation {
    float yaw   = 0.0f;   // rotation around Y (degrees)
    float pitch = 0.0f;   // rotation around X (degrees)
};

// One face of a multi-slab light field: one RGB image, its depth map, and
// the proxy mesh built from them.  Each slab also stores the viewing direction
// it was captured from so the compositor can select the right face at runtime.
class LightFieldSlab {
public:
    bool loadFromFiles(const std::string& rgbPath,
                       const std::string& depthPath,
                       const SlabOrientation& orient,
                       const std::string& label,
                       bool invertDepth);

    // Rebuild the proxy mesh using current depth and mesh settings.
    void rebuildMesh(int cols, int rows, float depthScale, float depthBias,
                     float tearThreshold, float backgroundCutoff,
                     InterpolationMode mode);

    void destroy();

    // Accessors ---------------------------------------------------------------
    const ImageResource& image() const { return image_; }
    ImageResource&       image()       { return image_; }
    const DepthMap&      depth() const { return depth_; }
    DepthMap&            depth()       { return depth_; }
    const DepthMesh&     mesh()  const { return mesh_; }
    const SlabOrientation& orientation() const { return orient_; }
    const std::string&   label() const { return label_; }
    bool valid() const { return image_.valid() && depth_.valid(); }
    bool meshReady() const { return mesh_.indexCount() > 0; }

    // Angular distance (in degrees) from this slab to a given camera direction.
    float angularDistance(float cameraYaw, float cameraPitch) const;

private:
    ImageResource   image_;
    DepthMap        depth_;
    DepthMesh       mesh_;
    SlabOrientation orient_;
    std::string     label_;
};

// Convenience: find the one or two closest slabs to the current camera angle.
struct SlabSelection {
    int primaryIdx   = -1;
    int secondaryIdx = -1;
    float blendWeight = 0.0f;   // 0 = all primary, 1 = all secondary
};

SlabSelection selectClosestSlabs(const std::vector<LightFieldSlab>& slabs,
                                 float cameraYaw, float cameraPitch);
