#pragma pack_matrix(row_major)

SamplerState linearSampler : register(s0);

cbuffer LightBuffer : register(b1) {
    float4 lightPos;
    float4 lightColor;
};

cbuffer MaterialBuffer : register(b4) {
    float4 baseColor;
    uint textureIndex;
    uint3 padding;
};

struct PSInput {
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float4 col : COLOR;
};

float4 PSMain(PSInput input) : SV_TARGET {
    Texture2D tex = ResourceDescriptorHeap[NonUniformResourceIndex(textureIndex)];
    float3 albedo = tex.Sample(linearSampler, input.uv).rgb * baseColor.rgb;

    float3 N = normalize(input.normal);
    float3 L = normalize(lightPos.xyz - input.worldPos);
    float NdotL = saturate(dot(N, L));
    
    float3 ambient = 0.2f * albedo;
    float3 diffuse = NdotL * lightColor.rgb * albedo;
    return float4(ambient + diffuse, 1.0f);
}