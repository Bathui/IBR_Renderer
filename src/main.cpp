#include <Application.h>

#include <string>

int main(int argc, char** argv) {
    const std::string inputPath = argc >= 2 ? argv[1] : "";
    return runInteractiveApp(inputPath);
}
