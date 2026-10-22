#pragma pack_matrix(row_major)

cbuffer TransformBuffer : register(b0) {
    float4x4 world;
    float4x4 view;
    float4x4 projection;
};

struct VSInput {
    float3 pos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float4 col : COLOR;
};

struct VSOutput {
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0; 
    float3 worldPos : TEXCOORD1;
    float4 col : COLOR;
};

VSOutput VSMain(VSInput input) {
    VSOutput output;

    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    output.worldPos = worldPos.xyz;
    output.pos = mul(mul(worldPos, view), projection);
    output.normal = normalize(mul(input.normal, (float3x3) world));
    output.uv = input.uv;
    output.col = input.col;
    return output;
}