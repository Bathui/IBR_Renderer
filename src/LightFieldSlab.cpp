#include <LightFieldSlab.h>

#include <algorithm>
#include <cmath>
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

bool LightFieldSlab::loadFromFiles(const std::string& rgbPath,
                                   const std::string& depthPath,
                                   const SlabOrientation& orient,
                                   const std::string& slabLabel,
                                   bool invertDepth) {
    orient_ = orient;
    label_  = slabLabel;

    if (!image_.loadWithTexture(rgbPath)) {
        std::cerr << "LightFieldSlab: failed to load RGB image: " << rgbPath << "\n";
        return false;
    }

    ImageResource depthImage;
    if (!depthImage.loadCpuOnly(depthPath)) {
        std::cerr << "LightFieldSlab: failed to load depth image: " << depthPath << "\n";
        return false;
    }

    depth_.loadFromImage(depthImage, invertDepth);
    depth_.updatePreviewTexture();

    if (!depth_.matchesSize(image_.width(), image_.height())) {
        std::cerr << "LightFieldSlab: depth dimensions do not match RGB for " << slabLabel << "\n";
        return false;
    }

    std::cout << "Loaded slab [" << slabLabel << "] from " << rgbPath << "\n";
    return true;
}

void LightFieldSlab::rebuildMesh(int cols, int rows, float depthScale,
                                 float depthBias, float tearThreshold,
                                 float backgroundCutoff,
                                 InterpolationMode /*mode*/) {
    if (!valid()) {
        return;
    }
    mesh_.rebuild(image_, depth_, cols, rows, depthScale, depthBias,
                  tearThreshold, backgroundCutoff);
}

void LightFieldSlab::destroy() {
    mesh_.destroy();
    depth_.destroyTexture();
    image_.destroyTexture();
}

float LightFieldSlab::angularDistance(float cameraYaw, float cameraPitch) const {
    // Convert both orientations to unit vectors on the sphere and compute the
    // great-circle angular distance between them.
    auto toDir = [](float yawDeg, float pitchDeg) {
        const float yr = static_cast<float>(yawDeg   * M_PI / 180.0);
        const float pr = static_cast<float>(pitchDeg * M_PI / 180.0);
        return glm::vec3(std::sin(yr) * std::cos(pr),
                         std::sin(pr),
                         std::cos(yr) * std::cos(pr));
    };

    const glm::vec3 camDir  = toDir(cameraYaw, cameraPitch);
    const glm::vec3 slabDir = toDir(orient_.yaw, orient_.pitch);

    const float dot = glm::clamp(glm::dot(camDir, slabDir), -1.0f, 1.0f);
    return static_cast<float>(std::acos(dot) * 180.0 / M_PI);
}

// ---------------------------------------------------------------------------
// Slab selection helper
// ---------------------------------------------------------------------------

SlabSelection selectClosestSlabs(const std::vector<LightFieldSlab>& slabs,
                                 float cameraYaw, float cameraPitch) {
    SlabSelection sel;
    if (slabs.empty()) {
        return sel;
    }

    // Compute angular distances and find the two closest slabs.
    struct Candidate {
        int   index;
        float distance;
    };
    std::vector<Candidate> candidates;
    candidates.reserve(slabs.size());
    for (int i = 0; i < static_cast<int>(slabs.size()); ++i) {
        if (slabs[i].valid()) {
            candidates.push_back({i, slabs[i].angularDistance(cameraYaw, cameraPitch)});
        }
    }
    if (candidates.empty()) {
        return sel;
    }

    std::sort(candidates.begin(), candidates.end(),
              [](const Candidate& a, const Candidate& b) { return a.distance < b.distance; });

    sel.primaryIdx = candidates[0].index;
    if (candidates.size() >= 2) {
        sel.secondaryIdx = candidates[1].index;
        const float totalDist = candidates[0].distance + candidates[1].distance;
        if (totalDist > 0.001f) {
            // Weight: 0 = all primary, 1 = all secondary.
            sel.blendWeight = candidates[0].distance / totalDist;
        }
    }

    return sel;
}
