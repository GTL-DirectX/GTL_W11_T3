
#include "ShaderRegisters.hlsl"

struct FMeshParticleInstanceVertex
{
    float4 Color; // COLOR0
    row_major float4x4 Transform; // TEXCOORD1~4
    float4 Velocity; // TEXCOORD5
    //uint2 SubUVParams[2]; // TEXCOORD6
    //float SubUVLerp; // TEXCOORD7
    //float RelativeTime; // TEXCOORD8
};



StructuredBuffer<FMeshParticleInstanceVertex> MeshInstances : register(t60);

struct VSInputMeshStatic
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 UV       : TEXCOORD0;
};

struct VSInputMeshInst
{
    float4 Color      : COLOR0;
    float4x4 Transform: TEXCOORD1;
    float4 Velocity   : TEXCOORD5;
    int4   SubUVParms : TEXCOORD6;
    float  SubUVLerp  : TEXCOORD7;
    float  RelTime    : TEXCOORD8;
};

struct VSOutputMesh
{
    float4 Position   : SV_POSITION;
    float3 Normal     : NORMAL;
    float2 UV         : TEXCOORD0;
    float4 Color      : COLOR0;
};

//VSOutputMesh mainVS_Mesh(VSInputMeshStatic VS, VSInputMeshInst I)
//{
//    VSOutputMesh OUT;
//    float4 worldPos = mul(float4(VS.Position,1), I.Transform);
//    OUT.Position = mul(worldPos, mul(ViewMatrix, ProjectionMatrix));
//    OUT.Normal   = normalize(mul(VS.Normal, (float3x3)I.Transform));
//    OUT.UV       = VS.UV;
//    OUT.Color    = I.Color;
//    return OUT;
//}

VSOutputMesh mainVS_Mesh(
    VSInputMeshStatic InStatic,
    uint InstanceID : SV_InstanceID
)
{
    VSOutputMesh Out;

    // 인스턴스 데이터를 SV_InstanceID 로 읽어오기
    FMeshParticleInstanceVertex I = MeshInstances[InstanceID];

    //float4 worldPos = float4(InStatic.Position, 1);
    float4 worldPos = mul(float4(InStatic.Position, 1), I.Transform);
    Out.Position = mul(worldPos, mul(ViewMatrix, ProjectionMatrix));

    Out.Normal = normalize(mul(InStatic.Normal, (float3x3) I.Transform));
    Out.UV = InStatic.UV;
    Out.Color = I.Color;
    return Out;
}


float4 mainPS_Mesh(VSOutputMesh IN) : SV_TARGET
{
    float4 tex = MaterialTextures[TEXTURE_SLOT_DIFFUSE].Sample(MaterialSamplers[TEXTURE_SLOT_DIFFUSE], IN.UV);
    //clip(tex.a - 0.01f);
    float3 N = normalize(IN.Normal);
    // 간단 디퓨즈 계산
    float3 lightDir = normalize(float3(0,0,-1));
    //float diff = max(dot(N, lightDir), 0);
    return float4(tex.rgb, 1.0f);
}
