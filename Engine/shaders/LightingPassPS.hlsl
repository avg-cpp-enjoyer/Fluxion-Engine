struct PSInput {
    float4 pos : SV_POSITION;
    float2 uv : TEXCOORD0;
};

cbuffer LightingParams : register(b5) {
    uint albedoIndex;
    uint normalIndex;
    uint materialIndex;
    uint padding;
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
    float4 material = materialTex.Load(pixelCoord);

    float3 N = normalize(normal.xyz + 0.00001f);
    float3 L = normalize(float3(1.0f, 1.0f, -1.0f));
    float NdotL = saturate(dot(N, L));
    
    float3 litColor = (0.2f * albedo.rgb) + (NdotL * lightColor.rgb * albedo.rgb);
    float isGeometry = step(0.1f, dot(normal.xyz, normal.xyz));
    float3 backgroundColor = float3(0.05f, 0.10f, 0.20f);
    float3 finalColor = lerp(backgroundColor, litColor, isGeometry);
    return float4(finalColor, 1.0f);
}