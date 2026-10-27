#pragma pack_matrix(row_major)

cbuffer MaterialBuffer : register(b4) {
    float4 baseColor;
    uint textureIndex;
    uint normalTextureIndex;
    uint roughnessTextureIndex;
    uint metallicTextureIndex;
    uint aoTextureIndex;
    uint3 padding;
};

SamplerState linearSampler : register(s0);

struct PSInput {
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TANGENT;
    float3 bitangent : BINORMAL;
};

struct PSOutput {
    float4 albedo : SV_TARGET0;
    float4 normal : SV_TARGET1;
    float4 material : SV_TARGET2;
};

PSOutput PSMain(PSInput input) {
    PSOutput output;

    Texture2D albedoTex = ResourceDescriptorHeap[textureIndex];
    float4 albedo = albedoTex.Sample(linearSampler, input.uv) * baseColor;
    output.albedo = float4(albedo.rgb, 1.0f);

    Texture2D normalTex = ResourceDescriptorHeap[normalTextureIndex];
    float3 tangentNormal = normalTex.Sample(linearSampler, input.uv).rgb * 2.0f - 1.0f;
    float3x3 TBN = float3x3(normalize(input.tangent), normalize(input.bitangent), normalize(input.normal));
    float3 worldNormal = normalize(mul(tangentNormal, TBN));
    
    if (dot(worldNormal, worldNormal) < 0.01f) {
        worldNormal = normalize(input.normal);
    }
        
    output.normal = float4(worldNormal, 1.0f);

    Texture2D roughTex = ResourceDescriptorHeap[roughnessTextureIndex];
    Texture2D metalTex = ResourceDescriptorHeap[metallicTextureIndex];
    Texture2D aoTex = ResourceDescriptorHeap[aoTextureIndex];

    float roughness = roughTex.Sample(linearSampler, input.uv).r;
    float metallic = metalTex.Sample(linearSampler, input.uv).r;
    float ao = aoTex.Sample(linearSampler, input.uv).r;

    output.material = float4(metallic, roughness, 0.0f, ao);
    return output;
}