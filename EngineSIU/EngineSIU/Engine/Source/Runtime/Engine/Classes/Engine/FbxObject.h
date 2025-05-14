#pragma once
#include "Define.h"
#include "Math/Quat.h"

// !!! 파싱할때의 자료구조로, 파서 이외에서는 사용하지 않습니다.
struct FFbxVertex
{
    FVector position;
    FLinearColor color;
    FVector normal;
    FVector4 tangent;
    FVector2D uv;
    int materialIndex = -1;
    int boneIndices[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };
    float boneWeights[8] = { 0,0,0,0,0,0,0,0 };
    //inline const static D3D11_INPUT_ELEMENT_DESC LayoutDesc[] = {
    //    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"MATERIAL_INDEX", 0, DXGI_FORMAT_R32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"BONE_INDICES", 0, DXGI_FORMAT_R8G8B8A8_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"BONE_INDICES", 1, DXGI_FORMAT_R8G8B8A8_SINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"BONE_WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //    {"BONE_WEIGHTS", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    //}; 
};
struct FFbxMeshData
{
    TArray<FFbxVertex> vertices;
    TArray<uint32> indices;
    TArray<uint32> subsetIndex;
    FString name;
};

struct FFbxJoint
{
    FString name;
    int parentIndex;
    FMatrix localBindPose;
    FMatrix inverseBindPose;
    FVector position;
    FQuat rotation;
    FVector scale;
};

struct FFbxSkeletonData
{
    TArray<FFbxJoint> joints;
};

struct FFbxSkeletalMesh {
    FString name;
    TArray<FFbxMeshData> mesh;
    FFbxSkeletonData skeleton;
    TArray<UMaterial*> material;
    TArray<FMaterialSubset> materialSubsets;
    FVector AABBmin, AABBmax;

    int GetBoneIndex(FString BoneName) const
    {
        for (int Idx = 0; Idx < skeleton.joints.Num(); ++Idx)
        {
            if (skeleton.joints[Idx].name == BoneName)
            {
                return Idx;
            }
        }
        return -1;
    }
};

// 기존 애니메이션 파싱을 UAnimSequence와 분리하기 위해서 Fbx정보를 그대로 파싱한 구조체를 따로 저장
struct FFbxAnimTrack // bone마다 == curve
{
    FString BoneName; // joint와는 name으로 연결되어있음. 이후 FBoneAnimationTrack으로 변환
    TArray<FVector> PosKeys;
    TArray<FQuat> RotKeys;
    TArray<FVector> ScaleKeys;
};

struct FFbxAnimSequence // 애니메이션 하나 == layer
{
    FString Name;
    float Duration;
    float FrameRate;
    int32 NumFrames;
    int32 NumKeys; // ????
    TArray<FFbxAnimTrack> AnimTracks; // 애니메이션 트랙
};
////////////////

struct FFbxAnimCurveKey
{
    enum class EInterpolationType
    {
        Constant = 0x00000002,
        Linear = 0x00000004,
        Cubic = 0x00000008,
    };
    float Time;
    float Value;
    FFbxAnimCurveKey::EInterpolationType Type;
    float ArriveTangent = 0.f;
    float LeaveTangent = 0.f;
};

struct FFbxAnimCurve
{
    TArray<FFbxAnimCurveKey> KeyFrames;
};

// 여러개(x,y,z, pitch, ...)의 curve를 저장한 구조체
struct FFbxCurveNode
{
    enum class ECurveChannel : uint8
    {
        TX = 0,
        TY = 1,
        TZ = 2,
        RX = 3,
        RY = 4,
        RZ = 5,
        SX = 6,
        SY = 7,
        SZ = 8,
    };
    TMap<ECurveChannel, FFbxAnimCurve> Curves;
};

struct FFbxAnimLayer
{
    TMap<FString, FFbxCurveNode> AnimCurves; // Bone Name -> CurveNode
};

// 여러개의 애니메이션
struct FFbxAnimStack
{
    FString Name;
    TMap<FString, FFbxAnimLayer> AnimLayers; // 하나의 애니메이션을 구성
    float FrameRate;
};

/*
1. FbxAnimStack
정의:
FBX 파일 내에 존재하는 하나의 "애니메이션 시퀀스" 또는 "애니메이션 클립"을 의미합니다.
역할:
여러 개의 AnimStack이 하나의 FBX 파일에 존재할 수 있으며, 각각은 독립적인 애니메이션(예: 걷기, 뛰기, 점프 등)을 표현합니다.
비유:
게임에서 애니메이션 클립(걷기/뛰기/점프 등) 하나하나가 AnimStack에 해당합니다.
2. FbxAnimLayer
정의:
AnimStack 내에서 실제 애니메이션 데이터(키프레임, 커브 등)를 담는 "레이어"입니다.
역할:
하나의 AnimStack 내에 여러 개의 AnimLayer를 둘 수 있습니다.
여러 AnimLayer를 합성해서(블렌딩 등) 최종 애니메이션 결과를 만들 수 있습니다.
일반적으로 FBX 파일에는 AnimStack 당 AnimLayer가 1개인 경우가 많지만, 여러 개도 가능합니다.
비유:
포토샵에서 여러 레이어를 겹쳐 최종 이미지를 만드는 것처럼, 여러 AnimLayer를 합칠 수 있습니다.
*/
