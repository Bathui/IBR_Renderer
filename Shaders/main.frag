#version 330 core
in vec2 vUv;
uniform sampler2D uImage;
out vec4 fragColor;
void main() {
    fragColor = vec4(texture(uImage, vUv).rgb, 1.0);
}
