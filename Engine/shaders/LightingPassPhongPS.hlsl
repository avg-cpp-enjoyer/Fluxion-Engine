#pragma pack_matrix(row_major)

#define EPSILON 1e-6f
#define GEOMETRY_THRESHOLD 0.1f
#define AMBIENT_INTENSITY 0.2f
#define DEFAULT_LIGHT_DIR float3(1.0f, 1.0f, -1.0f)
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

cbuffer LightBuffer : register(b1) {
    float4 lightPos;
    float4 lightColor;
};

float4 PSMain(PSInput input) : SV_TARGET {
    int3 pixelCoord = int3(input.pos.xy, 0);
   
    Texture2D<float4> albedoTex = ResourceDescriptorHeap[albedoIndex];
    Texture2D<float4> normalTex = ResourceDescriptorHeap[normalIndex];
    Texture2D<float4> materialTex = ResourceDescriptorHeap[materialIndex];

    float4 albedo = albedoTex.Load(pixelCoord);
    float4 normal = normalTex.Load(pixelCoord);

    float3 N = normalize(normal.xyz + EPSILON);
    float3 L = normalize(DEFAULT_LIGHT_DIR);
    float NdotL = saturate(dot(N, L));
    
    float3 litColor = (AMBIENT_INTENSITY * albedo.rgb) + (NdotL * lightColor.rgb * albedo.rgb);
    float isGeometry = step(GEOMETRY_THRESHOLD, dot(normal.xyz, normal.xyz));
    float3 finalColor = lerp(BACKGROUND_COLOR, litColor, isGeometry);
    return float4(finalColor, 1.0f);
}