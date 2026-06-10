#version 330 core
in vec2 vUv;
uniform usampler2D uIndexMap;
uniform sampler1D uCodebook;
out vec4 fragColor;
void main() {
    ivec2 indexSize = textureSize(uIndexMap, 0);
    vec2 p = vUv * vec2(indexSize) * 2.0;
    
    ivec2 p_int = ivec2(clamp(p, vec2(0.0), vec2(indexSize * 2) - 1.0));
    ivec2 b = p_int / 2;
    ivec2 offset = p_int % 2;
    
    uint k = texelFetch(uIndexMap, b, 0).r;
    int codebookIdx = int(k * 4u) + offset.y * 2 + offset.x;
    
    fragColor = vec4(texelFetch(uCodebook, codebookIdx, 0).rgb, 1.0);
}
