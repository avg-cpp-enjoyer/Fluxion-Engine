#pragma pack_matrix(row_major)

cbuffer TransformBuffer : register(b0) {
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

struct VSInput {
    float3 pos : POSITION;
};

struct VSOutput {
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD0;
};

VSOutput VSMain(VSInput input) {
    VSOutput output;
    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    output.pos = mul(mul(worldPos, view), projection);
    output.worldPos = worldPos.xyz;
    return output;
}