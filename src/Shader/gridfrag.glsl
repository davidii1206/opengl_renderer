#version 460 core
in vec3 vWorldPos;    
out vec4 FragColor;

uniform float uGridSize = 1; 
uniform float uMajorLineEvery = 10;
uniform float uThickness = 0.005;

void main() {
    vec2 coord = vWorldPos.xz;

    float dx = abs(fract(coord.x / uGridSize)) * uGridSize;
    float dz = abs(fract(coord.y / uGridSize)) * uGridSize;
    float minorDist = min(dx, dz);

    float dxMajor = abs(fract(coord.x / (uGridSize * uMajorLineEvery))) * uGridSize * uMajorLineEvery;
    float dzMajor = abs(fract(coord.y / (uGridSize * uMajorLineEvery))) * uGridSize * uMajorLineEvery;
    float majorDist = min(dxMajor, dzMajor);

    float minorAlpha = 1.0 - smoothstep(0.0, fwidth(minorDist), minorDist);
    float majorAlpha = 1.0 - smoothstep(0.0, fwidth(majorDist), majorDist);

    float intensity = max(0.3 * minorAlpha, 0.8 * majorAlpha);

    if (intensity < 0.01) discard;

    FragColor = vec4(vec3(intensity), 1.0);
}
