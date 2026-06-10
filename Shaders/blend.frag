#version 330 core
in vec2 vUv;
uniform sampler2D uSlabA;
uniform sampler2D uSlabB;
uniform sampler2D uDepthA;
uniform sampler2D uDepthB;
uniform float uBlendWeight;
out vec4 fragColor;
void main() {
    vec4 a = texture(uSlabA, vUv);
    vec4 b = texture(uSlabB, vUv);
    float dA = texture(uDepthA, vUv).r;
    float dB = texture(uDepthB, vUv).r;
    
    if (a.a > 0.0 && b.a > 0.0) {
        float depthDiff = dB - dA;
        float bias = 0.01; // Depth tolerance
        if (depthDiff > bias) {
            fragColor = a; // A is much closer, occludes B
        } else if (depthDiff < -bias) {
            fragColor = b; // B is much closer, occludes A
        } else {
            fragColor = vec4(mix(a.rgb, b.rgb, uBlendWeight), 1.0);
        }
    } else if (a.a > 0.0) {
        fragColor = a;
    } else if (b.a > 0.0) {
        fragColor = b;
    } else {
        fragColor = vec4(a.rgb, 0.0); // Both background
    }
}
