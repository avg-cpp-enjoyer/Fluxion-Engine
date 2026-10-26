#pragma pack_matrix(row_major)

cbuffer MaterialBuffer : register(b4) {
    float4 baseColor;
    uint textureIndex;
    uint3 padding;
};

SamplerState linearSampler : register(s0);

struct PSInput {
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float4 col : COLOR;
};

struct PSOutput {
    float4 albedo : SV_TARGET0; 
    float4 normal : SV_TARGET1; 
    float4 material : SV_TARGET2;
};

PSOutput PSMain(PSInput input) {
    PSOutput output;

    Texture2D tex = ResourceDescriptorHeap[NonUniformResourceIndex(textureIndex)];
    float4 texColor = tex.Sample(linearSampler, input.uv) * baseColor;

    output.albedo = float4(texColor.rgb, 1.0f); //temporarily hardcoded AO = 1.0 
    output.normal = float4(normalize(input.normal), 1.0f);
    output.material = float4(0.0f, 0.5f, 0.0f, 1.0f); //temporarily hardcoded
    return output;
}