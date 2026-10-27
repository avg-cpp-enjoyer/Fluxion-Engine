#pragma pack_matrix(row_major)

struct VSOutput {
    float4 pos : SV_POSITION;
};

VSOutput VSMain(uint vertexID : SV_VertexID) {
    VSOutput output;
    
    float2 uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.pos = float4(uv * 2.0f - 1.0f, 0.0f, 1.0f);
    output.pos.y = -output.pos.y;
    
    return output;
}