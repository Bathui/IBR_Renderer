# Depthy-Style IBR Viewer

This project is a real-time Depth Image-Based Rendering prototype. It loads a single image, generates or edits a depth map, turns the RGB-D pair into a textured proxy mesh, and renders small camera-angle changes in OpenGL.

## Build

```powershell
cmake --build build --config release
.\build\Release\IBR_Viewer.exe
```

The interactive app starts empty. Enter an RGB image path and a matching grayscale depth image path in the ImGui panel, then click **Load** and **Load depth**.

The bundled `libs/glfw3.lib` is an MSVC library, so use Visual Studio/MSVC rather than MinGW for this repo.

## Controls

- **Depth input**: load a grayscale depth image where white is near and black is far.
- **Generated depth fallback**: optional reset/generation, invert, luminance influence, smoothing, depth scale, and bias.
- **Depth painting**: left-drag on the depth preview to paint toward the selected depth value; right-drag to smooth locally.
- **Warp view**: yaw, pitch, zoom, pan, perspective strength, mesh resolution, and wireframe toggle.

## Good Test Image Types

- A portrait or close foreground subject: easiest to see face/object bulging forward.
- A still life with objects at different depths: useful for painting foreground/background separation.
- A street or room scene: shows larger parallax, but also reveals holes and stretching more quickly.

Useful references for the writeup:

- Depthy: https://depthy.stamina.pl/
- Depth Image Based Rendering artifact examples: https://alregib.ece.gatech.edu/demos/depth-image-based-rendering/
- Shih et al. 2020, 3D Photography using Context-Aware Layered Depth Inpainting: https://shihmengli.github.io/3D-Photo-Inpainting/
