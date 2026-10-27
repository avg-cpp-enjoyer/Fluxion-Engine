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
    float4 color : COLOR;
    float4 tangent : TANGENT;
};

struct VSOutput {
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
    float3 tangent : TANGENT; 
    float3 bitangent : BINORMAL;
};

VSOutput VSMain(VSInput input) {
    VSOutput output;
    
    float4 worldPos = mul(float4(input.pos, 1.0f), world);
    output.pos = mul(mul(worldPos, view), projection);
    output.worldPos = worldPos.xyz;
    
    float3x3 world3x3 = (float3x3) world;
    float3 N = normalize(mul(input.normal, world3x3));
    float3 T = normalize(mul(input.tangent.xyz, world3x3));
    float3 B = cross(N, T) * input.tangent.w;
    
    output.normal = N;
    output.tangent = T;
    output.bitangent = B;
    output.uv = input.uv;
    return output;
}