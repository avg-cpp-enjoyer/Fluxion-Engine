#pragma pack_matrix(row_major)

#define PI 3.14159265f
#define EPSILON 1e-6f    
#define GGX_DENOM_EPSILON 1e-7f
#define SPEC_DENOM_EPSILON 0.0001f  
#define DIELECTRIC_F0 0.04f     
#define AMBIENT_INTENSITY 0.03f     
#define GEOMETRY_THRESHOLD 0.1f      
#define BACKGROUND_COLOR float3(0.05f, 0.10f, 0.20f) 

struct PSInput {
    float4 pos : SV_POSITION;
};

cbuffer LightingParams : register(b5) {
    uint albedoIndex;
    uint normalIndex;
    uint materialIndex;
    uint depthIndex;
    float screenWidth;
    float screenHeight;
    float2 padding;
};

cbuffer CameraParams : register(b3) {
    float4 cameraPos;
    float4x4 invViewProj;
};

cbuffer LightBuffer : register(b1) {
    float4 lightPos;
    float4 lightColor;
};

float3 FresnelSchlick(float cosTheta, float3 F0) {
    return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float DistributionGGX(float3 N, float3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = saturate(dot(N, H));
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / max(denom, GGX_DENOM_EPSILON);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0; 
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness) {
    float NdotV = saturate(dot(N, V));
    float NdotL = saturate(dot(N, L));
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}

float4 PSMain(PSInput input) : SV_TARGET {
    int3 pixelCoord = int3(input.pos.xy, 0);
    
    Texture2D<float4> albedoTex = ResourceDescriptorHeap[albedoIndex];
    Texture2D<float4> normalTex = ResourceDescriptorHeap[normalIndex];
    Texture2D<float4> materialTex = ResourceDescriptorHeap[materialIndex];
    Texture2D<float> depthTex = ResourceDescriptorHeap[depthIndex];

    float4 albedo = albedoTex.Load(pixelCoord);
    float4 normal = normalTex.Load(pixelCoord);
    float4 material = materialTex.Load(pixelCoord);
    float depth = depthTex.Load(pixelCoord);

    if (depth >= 1.0f || dot(normal.xyz, normal.xyz) < GEOMETRY_THRESHOLD) {
        return float4(BACKGROUND_COLOR, 1.0f);
    }

    float2 ndc;
    ndc.x = (input.pos.x / screenWidth) * 2.0f - 1.0f;
    ndc.y = 1.0f - (input.pos.y / screenHeight) * 2.0f;
    
    float4 clipPos = float4(ndc, depth, 1.0f);
    float4 worldPos = mul(clipPos, invViewProj);
    worldPos.xyz /= max(abs(worldPos.w), EPSILON);
    
    float3 N = normalize(normal.xyz + EPSILON);
    float3 V = normalize(cameraPos.xyz - worldPos.xyz + EPSILON);
    float3 L = normalize(lightPos.xyz - worldPos.xyz + EPSILON);
    float3 H = normalize(V + L + EPSILON);

    float metallic = material.r;
    float roughness = material.g;
    float3 F0 = lerp(float3(DIELECTRIC_F0, DIELECTRIC_F0, DIELECTRIC_F0), albedo.rgb, metallic);

    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    float3 F = FresnelSchlick(saturate(dot(H, V)), F0);

    float3 numerator = NDF * G * F;
    float denominator = 4.0f * saturate(dot(N, V)) * saturate(dot(N, L)) + SPEC_DENOM_EPSILON;
    float3 specular = numerator / denominator;

    float3 kS = F;
    float3 kD = (1.0f - kS) * (1.0f - metallic);
    float NdotL = saturate(dot(N, L));

    float3 ambient = AMBIENT_INTENSITY * albedo.rgb;
    float3 radiance = lightColor.rgb;
    float3 Lo = (kD * albedo.rgb / PI + specular) * radiance * NdotL;

    float3 finalColor = ambient + Lo;
    finalColor = finalColor / (finalColor + 1.0f);    
    return float4(finalColor, 1.0f);
}