#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Components/Material/Material.h"
#include "Engine/Asset/SkeletalMeshAsset.h"
#include "Define.h"

class USkeletalMesh : public UObject
{
    DECLARE_CLASS(USkeletalMesh, UObject)

public:
    USkeletalMesh() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    const TArray<UMaterial*>& GetMaterials() const;
    uint32 GetMaterialIndex(FString MaterialSlotName) const;
    void GetUsedMaterials(TArray<UMaterial*>& OutMaterial) const;
    const FSkeletalMeshRenderData& GetRenderData() const { return RenderData; }
    void GetRefSkeleton(FReferenceSkeleton& OutRefSkeleton) const;
    void GetInverseBindPoseMatrices(TArray<FMatrix>& OutMatrices) const;

    void SetCPUSkinned(bool bInCPUSkinned);
    bool GetCPUSkinned() const;

    //ObjectName은 경로까지 포함
    FString GetObjectName() const;

    void SetData(FSkeletalMeshRenderData InRenderData,
        FReferenceSkeleton InRefSkeleton,
        TArray<FMatrix> InInverseBindPoseMatrices,
        TArray<UMaterial*> InMaterials);

private:
    bool bCPUSkinned : 1 = false;
private:
    // TODO : UPROPERTIES
    // FbxLoader에 저장되어있음
    // 이건 수정하면 안됨(CPU Skinning에서도 이걸로 수정하면 안됨)
    FSkeletalMeshRenderData RenderData;

    // 로드 당시의 Skeleton로 초기화. Matrix는 매번 생성해서 사용
    FReferenceSkeleton RefSkeleton;

    // Inverse Bind Pose Matrix : const로 사용
    TArray<FMatrix> InverseBindPoseMatrices;

    // FSkeletalMaterial를 만들 이유가 없음...
    TArray<UMaterial*> Materials;

};
