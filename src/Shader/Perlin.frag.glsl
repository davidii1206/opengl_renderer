#version 460 core

out vec4 FragColor;
in float vHeight;
in vec2 vTexCoord;

void main() {
    vec3 baseColor = vec3(vHeight, vHeight, vHeight);
    
    FragColor = vec4(baseColor, 1.0);
}