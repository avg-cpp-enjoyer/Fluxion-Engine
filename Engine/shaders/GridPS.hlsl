#pragma pack_matrix(row_major)

struct VSOutput {
    float4 pos : SV_POSITION;
    float3 worldPos : TEXCOORD0;
};

cbuffer GridParams : register(b2) {
    float cellSize;
    float lineWidth;
    float baseAlpha;
    float padding1;
    float3 gridColor;
    float padding2;
    float3 axisXColor;
    float padding3;
    float3 axisZColor;
    float padding4;
    float axisWidth;
    float fadeDistance;
    float padding5[2];
};

cbuffer CameraParams : register(b3) {
    float3 cameraPos;
    float pad1;
};

float aastep(float threshold, float x, float fw) {
    return smoothstep(threshold - fw, threshold + fw, x);
}

float ddLength(float a) {
    return length(float2(ddx(a), ddy(a)));
}

float pristineGrid(float2 uv, float2 lineWidth) {
    lineWidth = saturate(lineWidth);
    float4 uvDDXY = float4(ddx(uv), ddy(uv));
    float2 uvDeriv = float2(length(uvDDXY.xz), length(uvDDXY.yw));
    bool2 invertLine = lineWidth > 0.5;
    float2 targetWidth = select(invertLine, 1.0 - lineWidth, lineWidth);
    float2 drawWidth = clamp(targetWidth, uvDeriv, 0.5);
    float2 lineAA = max(uvDeriv, 0.000001) * 1.5;
    float2 gridUV = abs(frac(uv) * 2.0 - 1.0);
    gridUV = select(invertLine, gridUV, 1.0 - gridUV);
    float2 grid2 = smoothstep(drawWidth + lineAA, drawWidth - lineAA, gridUV);
    grid2 *= saturate(targetWidth / drawWidth);
    grid2 = lerp(grid2, targetWidth, saturate(uvDeriv * 2.0 - 1.0));
    grid2 = select(invertLine, 1.0 - grid2, grid2);
    return lerp(grid2.x, 1.0, grid2.y);
}

float4 PSMain(VSOutput input) : SV_TARGET {
    float2 uv = input.worldPos.xz / cellSize;
    float lineMask = pristineGrid(uv, float2(lineWidth, lineWidth));

    float ax = 1.0 - aastep(lineWidth, abs(input.worldPos.z) / cellSize, ddLength(input.worldPos.z / cellSize));
    float az = 1.0 - aastep(lineWidth, abs(input.worldPos.x) / cellSize, ddLength(input.worldPos.x / cellSize));

    float3 col = gridColor;
    col = lerp(col, axisXColor, ax);
    col = lerp(col, axisZColor, az);

    float dist = distance(input.worldPos, cameraPos);
    float fade = saturate(1.0 - dist / fadeDistance);
    float alpha = lineMask * baseAlpha * fade;

    return float4(col, alpha);
}