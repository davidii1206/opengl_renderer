#version 460 core

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;
in vec3 Tangent;
in vec3 Bitangent;
in mat3 TBN;

out vec4 FragColor;

// Material properties
uniform vec4 u_baseColorFactor = vec4(1.0, 1.0, 1.0, 1.0);
uniform float u_metallicFactor = 1.0;
uniform float u_roughnessFactor = 1.0;
uniform vec3 u_emissiveFactor = vec3(0.0);
uniform float u_normalScale = 1.0;
uniform float u_occlusionStrength = 1.0;

// Textures
uniform sampler2D u_baseColorTexture;
uniform sampler2D u_metallicRoughnessTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_emissiveTexture;
uniform sampler2D u_occlusionTexture;

// Lighting
uniform vec3 u_lightDirection = vec3(-1.0, -1.0, -1.0);
uniform vec3 u_lightColor = vec3(1.0, 1.0, 1.0);
uniform vec3 u_cameraPosition;

// Texture availability flags (set by material system)
uniform bool u_hasBaseColorTexture = false;
uniform bool u_hasMetallicRoughnessTexture = false;
uniform bool u_hasNormalTexture = false;
uniform bool u_hasEmissiveTexture = false;
uniform bool u_hasOcclusionTexture = false;

vec3 getNormalFromMap() {
    if (!u_hasNormalTexture) {
        return normalize(Normal);
    }
    
    vec3 tangentNormal = texture(u_normalTexture, TexCoords).xyz * 2.0 - 1.0;
    tangentNormal.xy *= u_normalScale;
    
    return normalize(TBN * tangentNormal);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    
    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.14159265359 * denom * denom;
    
    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    
    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    
    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);
    
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    // Sample textures
    vec4 baseColor = u_baseColorFactor;
    if (u_hasBaseColorTexture) {
        baseColor *= texture(u_baseColorTexture, TexCoords);
    }
    
    float metallic = u_metallicFactor;
    float roughness = u_roughnessFactor;
    if (u_hasMetallicRoughnessTexture) {
        vec3 mr = texture(u_metallicRoughnessTexture, TexCoords).rgb;
        metallic *= mr.b; // Blue channel
        roughness *= mr.g; // Green channel
    }
    
    vec3 emissive = u_emissiveFactor;
    if (u_hasEmissiveTexture) {
        emissive *= texture(u_emissiveTexture, TexCoords).rgb;
    }
    
    float occlusion = 1.0;
    if (u_hasOcclusionTexture) {
        occlusion = texture(u_occlusionTexture, TexCoords).r;
        occlusion = 1.0 + u_occlusionStrength * (occlusion - 1.0);
    }
    
    // Get normal
    vec3 N = getNormalFromMap();
    vec3 V = normalize(u_cameraPosition - FragPos);
    vec3 L = normalize(-u_lightDirection);
    vec3 H = normalize(V + L);
    
    // Calculate reflectance
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, baseColor.rgb, metallic);
    
    // Cook-Torrance BRDF
    float NDF = distributionGGX(N, H, roughness);
    float G = geometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    
    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;
    
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    
    float NdotL = max(dot(N, L), 0.0);
    vec3 Lo = (kD * baseColor.rgb / 3.14159265359 + specular) * u_lightColor * NdotL;
    
    // Ambient lighting (very basic)
    vec3 ambient = vec3(0.4) * baseColor.rgb * occlusion;
    
    vec3 color = ambient + Lo + emissive;
    
    // HDR tonemapping (simple Reinhard)
    color = color / (color + vec3(1.0));
    
    // Gamma correction
    color = pow(color, vec3(1.0/2.2));
    
    FragColor = vec4(color, baseColor.a);
}