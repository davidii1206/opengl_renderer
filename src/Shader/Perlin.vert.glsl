#version 460 core

layout(location = 0) in vec3 aPos;

layout(std140, binding = 0) uniform CameraData {
    mat4 view;
    mat4 projection;
};

uniform float uTime;
uniform sampler2D uTexture;
uniform float uHeightScale;
uniform float uScrollSpeed;
uniform float uSpacing;
uniform float uCubeSize;
uniform int uTextureSize;

out float vHeight;
out vec2 vTexCoord;

// Bilinear interpolation function
float sampleHeightSmooth(vec2 uv) {
    vec2 textureSize = vec2(float(uTextureSize));
    
    // Convert UV to pixel coordinates
    vec2 pixelCoord = uv * textureSize - 0.5;
    
    // Get the four surrounding pixel coordinates
    ivec2 c00 = ivec2(floor(pixelCoord));
    ivec2 c10 = c00 + ivec2(1, 0);
    ivec2 c01 = c00 + ivec2(0, 1);
    ivec2 c11 = c00 + ivec2(1, 1);
    
    // Handle wrapping for seamless tiling
    c00 = ivec2(c00.x % uTextureSize, c00.y % uTextureSize);
    c10 = ivec2(c10.x % uTextureSize, c10.y % uTextureSize);
    c01 = ivec2(c01.x % uTextureSize, c01.y % uTextureSize);
    c11 = ivec2(c11.x % uTextureSize, c11.y % uTextureSize);
    
    // Sample the four corners
    float h00 = texelFetch(uTexture, c00, 0).r;
    float h10 = texelFetch(uTexture, c10, 0).r;
    float h01 = texelFetch(uTexture, c01, 0).r;
    float h11 = texelFetch(uTexture, c11, 0).r;
    
    // Calculate interpolation weights
    vec2 f = fract(pixelCoord);
    
    // Bilinear interpolation
    float h0 = mix(h00, h10, f.x);
    float h1 = mix(h01, h11, f.x);
    return mix(h0, h1, f.y);
}

// Alternative: Hardware-accelerated smooth sampling
float sampleHeightHardware(vec2 uv) {
    // Let the GPU handle interpolation automatically
    return texture(uTexture, uv).r;
}

void main()
{
    // Calculate grid position from instance ID
    int instanceID = gl_InstanceID;
    int gridSize = uTextureSize;
    
    int x = instanceID % gridSize;
    int y = instanceID / gridSize;
    
    // Convert grid coordinates to world position with spacing
    vec3 instancePos = vec3(
        (float(x) - float(gridSize) * 0.5) * uSpacing,
        0.0,
        (float(y) - float(gridSize) * 0.5) * uSpacing
    );
    
    // Calculate UV coordinates for this instance
    vec2 instanceUV = vec2(float(x) / float(gridSize), float(y) / float(gridSize));
    
    // Add scrolling offset
    vec2 scrollingUV = instanceUV + vec2(uTime * uScrollSpeed, 0.0);
    
    // Use smooth interpolated sampling
    float height = sampleHeightSmooth(scrollingUV);
    
    // Alternative: Use hardware interpolation (simpler, faster)
    // float height = sampleHeightHardware(scrollingUV);
    
    // Scale the cube vertices
    vec3 scaledPos = aPos * uCubeSize;
    
    // Apply height displacement
    instancePos.y += height * uHeightScale;
    
    // Final position
    vec3 worldPos = scaledPos + instancePos;
    
    // Pass data to fragment shader
    vHeight = height;
    vTexCoord = instanceUV;
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
}