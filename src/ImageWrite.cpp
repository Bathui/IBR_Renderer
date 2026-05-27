#include <ImageWrite.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

bool writePngRgba(const std::string& path, int width, int height, const std::vector<unsigned char>& rgba) {
    if (width <= 0 || height <= 0 || rgba.size() < static_cast<size_t>(width * height * 4)) {
        return false;
    }
    return stbi_write_png(path.c_str(), width, height, 4, rgba.data(), width * 4) != 0;
}
