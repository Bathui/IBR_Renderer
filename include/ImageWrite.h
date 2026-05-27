#pragma once

#include <string>
#include <vector>

bool writePngRgba(const std::string& path, int width, int height, const std::vector<unsigned char>& rgba);
