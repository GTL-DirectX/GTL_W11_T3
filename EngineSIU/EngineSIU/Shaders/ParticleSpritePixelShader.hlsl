// TODO: 현재는 머티리얼에서 디퓨즈만 전달 받는 상태로 진행. 추후 머티리얼 전체를 전달 받는 방법 고려하면 좋음.
#include "ShaderRegisters.hlsl"

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

cbuffer MaterialConstants : register(b0)
{
    FMaterial Material;
}

cbuffer SubUVConstant : register(b1)
{
    float2 UVOffset;
    float2 UVScale; // sub UV 셀의 크기 (예: 1/CellsPerColumn, 1/CellsPerRow)
}

struct PS_Input
{
    float4 Position : SV_Position;
    float2 UV : TEXCOORD0;
    float4 Color : COLOR0;
    float RelativeTime : TEXCOORD1;
    float ParticleId : TEXCOORD2;
    float SubImageIndex : TEXCOORD3;
};

float4 mainPS(PS_Input input) : SV_TARGET
{
    // float2 UV = input.UV * UVScale + UVOffset;
    // float DiffuseColor = Texture.Sample(Sampler, UV);
    // float4 FinalColor = DiffuseColor * input.Color;
    // float4 Color = Texture.Sample(Sampler, UV);

    float4 tex = MaterialTextures[TEXTURE_SLOT_DIFFUSE]
                 .Sample(MaterialSamplers[TEXTURE_SLOT_DIFFUSE], input.UV);
    clip(tex.a - 0.1f);
    float3 albedo = tex.rgb * Material.DiffuseColor * input.Color.rgb;
    float  alpha  = tex.a   * Material.Opacity     * input.Color.a;

    return float4(albedo, 1.0f);
}
