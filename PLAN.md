# Depthy-Style Milestone 1 Plan

## Summary
Build a real-time single-image 3D warping demo inspired by [Depthy](https://depthy.stamina.pl/): load one RGB image, create/edit a grayscale depth map, convert RGB + depth into a textured proxy mesh, and use ImGui controls to view the image from slightly different camera angles.

This is a good Milestone 1 MVP because it gives visible progress quickly while still connecting to image-based rendering and depth-image-based rendering ideas.

## Key Modules
- **App Shell / Build**
  - Add a minimal C++ OpenGL app using the existing GLFW, GLAD, ImGui, and GLM files.
  - Add `CMakeLists.txt` or a simple documented build path.
  - Start with one window containing three views: original image, depth map, warped render.

- **Image I/O Module**
  - Use `stb_image` for loading `.jpg/.png` input images.
  - Default input: `Input_Images/selfie.jpg`.
  - Store image data as CPU `uint8 RGB/RGBA` plus one OpenGL texture.

- **Depth Map Module**
  - Maintain a CPU `float depth[w*h]` in `[0, 1]`, with `1 = near`, `0 = far`.
  - Milestone default depth generation:
    - center/portrait-style bump,
    - optional luminance influence,
    - smoothing slider.
  - Add ImGui controls: reset depth, invert depth, depth scale, depth bias.
  - Add a depth preview texture updated when depth changes.

- **Depth Painting Module**
  - Implement a simple brush over the image/depth preview.
  - Controls: brush radius, target depth value, brush strength.
  - Left-drag paints depth toward the selected value.
  - Right-drag smooths local depth.
  - This is the “Depthy-style” interactive part and should be visibly demoable.

- **Mesh Builder Module**
  - Convert the image into a regular grid mesh, default around `160 x 120` vertices or aspect-preserving equivalent.
  - Each vertex stores position `(x, y, z)` and texture coordinate `(u, v)`.
  - `z = depth * depthScale + depthBias`.
  - Rebuild or update vertex buffer when depth parameters or painted depth changes.
  - Support mesh resolution slider for quality/performance comparison.

- **Warp Renderer Module**
  - Render the textured mesh using OpenGL shaders.
  - Camera controls through ImGui:
    - yaw,
    - pitch,
    - zoom,
    - pan X/Y,
    - perspective strength.
  - Add optional wireframe toggle to show the proxy geometry.
  - Background color fills holes/disoccluded regions.

- **Milestone Demo / Writeup Artifacts**
  - Produce screenshots of:
    - original input image,
    - generated/painted depth map,
    - warped image at a side angle,
    - optional wireframe view.
  - Record a short video/gif showing real-time camera movement and depth painting.

## Testing And Progress Checkpoints
- **Checkpoint 1: Image Load**
  - App opens and displays `selfie.jpg` correctly as an OpenGL texture.
  - Test with another `.jpg` or `.png`.

- **Checkpoint 2: Depth Preview**
  - Generated depth map appears in the UI.
  - Sliders visibly change the grayscale depth preview.

- **Checkpoint 3: Mesh Warp**
  - The image appears as a textured 3D surface.
  - Yaw/pitch sliders create a visible 3D warp in real time.

- **Checkpoint 4: Painting**
  - Painting on the depth preview changes the depth map.
  - The warped mesh updates immediately after painting.

- **Checkpoint 5: Demo Quality**
  - At least one screenshot clearly resembles the “3D warp” effect.
  - Wireframe/debug view confirms the image is actually displaced by depth.

## Assumptions And Defaults
- Use the existing C++/OpenGL/ImGui project structure.
- Use single-image RGB-D style warping for Milestone 1, not a full Levoy-Hanrahan light-field viewer yet.
- Treat `stb_image` as a utility dependency, not the core project implementation.
- Keep camera motion small; large angle changes will reveal holes/stretching, which is expected for single-image depth warping.
- In the writeup, describe this honestly as a **Depth Image-Based Rendering / 3D photo prototype**, with future work connecting it to multi-view light-field rendering and better hole filling/inpainting.

## References To Mention
- Depthy: interactive depth-map based 3D photo/parallax viewer.
- Depth Image Based Rendering examples: image warping creates holes/disocclusions.
- Shih et al. 2020, “3D Photography using Context-Aware Layered Depth Inpainting”: future-work reference for filling missing regions.
