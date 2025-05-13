#pragma once
#include <fbxsdk.h>

#include "FbxObject.h"
#include "Container/Array.h"
#include "Container/Map.h"
#include "Container/String.h"
#include "Container/Queue.h"
#include "Asset/SkeletalMeshAsset.h"
#include "Animation/AnimSequence.h"

struct FFbxSkeletalMesh;
struct BoneWeights;
class USkeletalMesh;

// FFbxManager 전용으로만 사용
struct FFbxLoader
{
public:
    static void Init();
    //static void LoadFBX(const FString& filename);
    //static USkeletalMesh* GetSkeletalMesh(const FString& filename);
    static bool ParseFBX(const FString& FBXFilePath, 
        FFbxSkeletalMesh* OutFbxSkeletalMesh, 
        TArray<FFbxAnimSequence*>& OutFbxSequences,
        TArray<FFbxAnimStack*>& OutAnimStacks
    );
    static void GenerateSkeletalMesh(const FFbxSkeletalMesh* InFbxSkeletal, USkeletalMesh*& OutSkeletalMesh);
    static void GenerateAnimations(
        const FFbxSkeletalMesh* InFbxSkeletal,
        const FFbxAnimSequence* InFbxAnimSequence,
        FString& OutAnimName,
        UAnimSequence*& OutAnimSequence
        //const TArray<FFbxAnimSequence*>& InFbxAnimSequences,
        //const TArray<FFbxAnimStack*>& InFbxAnimStacks,
        //TArray<FString>& OutAnimNames,
        //TArray<UAnimDataModel*>& OutAnimData
    );
    //static void ParseFBXAnimationOnly(
    //    const FString& filename, USkeletalMesh* skeletalMesh,
    //    TArray<UAnimSequence*>& OutSequences
    //);
private:
    static FbxIOSettings* GetFbxIOSettings();
    static FbxCluster* FindClusterForBone(FbxNode* boneNode);
    static void LoadFBXObject(FbxScene* InFbxScene, FFbxSkeletalMesh* OutFbxSkeletalMesh);
    static void LoadFbxSkeleton(
        FFbxSkeletalMesh* fbxObject,
        FbxNode* node,
        TMap<FString, int>& boneNameToIndex,
        int parentIndex
    );
    static void LoadSkinWeights(
        FbxNode* node,
        const TMap<FString, int>& boneNameToIndex,
        TMap<int, TArray<BoneWeights>>& OutBoneWeights
    );
    static void LoadFBXMesh(
        FFbxSkeletalMesh* fbxObject,
        FbxNode* node,
        TMap<FString, int>& boneNameToIndex,
        TMap<int, TArray<BoneWeights>>& boneWeight
    );
    static void LoadFBXAnimations(
        FbxScene* Scene,
        FFbxSkeletalMesh* fbxObject,
        TArray<FFbxAnimSequence*>& OutSequences
    );
    static void LoadFBXCurves(
        FbxScene* Scene,
        FFbxSkeletalMesh* fbxObject,
        TArray<FFbxAnimStack*>& OutAnimStacks
    );

    static void GetKeys(FbxAnimCurve* Curve, TArray<FFbxAnimCurveKey>& OutKeys);


    //static void LoadFBXAnimations(
    //    FFbxSkeletalMesh* fbxObject,
    //    FbxScene* scene,
    //    TArray<FFbxAnimTrack>& OutFbxAnimTracks
    //);


    //static void LoadAnimationData(
    //    FbxScene* Scene, FbxNode* RootNode, 
    //    USkeletalMesh* SkeletalMesh, UAnimSequence* OutSequence
    //);

    static void DumpAnimationDebug(const FString& FBXFilePath, const USkeletalMesh* SkeletalMesh, const TArray<UAnimSequence*>& AnimSequences);

    static bool CreateTextureFromFile(const FWString& Filename, bool bIsSRGB);
    static void LoadFBXMaterials(
        FFbxSkeletalMesh* fbxObject,
        FbxNode* node
    );
    static void CalculateTangent(FFbxVertex& PivotVertex, const FFbxVertex& Vertex1, const FFbxVertex& Vertex2);
    static FbxNode* FindBoneNode(FbxNode* Root, const FString& BoneName);
    static FTransform FTransformFromFbxMatrix(const FbxAMatrix& Matrix);
    //inline static TArray<FSkeletalMeshRenderData> RenderDatas; // 일단 Loader에서 가지고 있게 함

    inline static FbxManager* Manager;
public:
    static const FbxAxisSystem UnrealTargetAxisSystem;
    inline static const FQuat FinalBoneCorrectionQuat = FQuat(FVector(0, 0, 1), FMath::DegreesToRadians(-90.0f));
};
