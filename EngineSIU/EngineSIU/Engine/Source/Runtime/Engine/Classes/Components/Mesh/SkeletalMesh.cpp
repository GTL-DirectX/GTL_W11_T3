#include "SkeletalMesh.h"

UObject* USkeletalMesh::Duplicate(UObject* InOuter)
{

    // TODO
    return nullptr;
}

const TArray<UMaterial*>& USkeletalMesh::GetMaterials() const
{
    return Materials;
}

uint32 USkeletalMesh::GetMaterialIndex(FString MaterialSlotName) const
{
    for (uint32 materialIndex = 0; materialIndex < Materials.Num(); materialIndex++) {
        if (Materials[materialIndex]->GetMaterialInfo().MaterialName == MaterialSlotName)
            return materialIndex;
    }

    return -1;
}

void USkeletalMesh::GetUsedMaterials(TArray<UMaterial*>& OutMaterial) const
{
    for (UMaterial* Material : Materials)
    {
        OutMaterial.Emplace(Material);
    }
}

void USkeletalMesh::GetRefSkeleton(FReferenceSkeleton& OutRefSkeleton) const
{
    OutRefSkeleton = RefSkeleton;
}

void USkeletalMesh::GetInverseBindPoseMatrices(TArray<FMatrix>& OutMatrices) const
{
    OutMatrices = InverseBindPoseMatrices;
}

void USkeletalMesh::SetCPUSkinned(bool bInCPUSkinned)
{
    bCPUSkinned = bInCPUSkinned;
}

bool USkeletalMesh::GetCPUSkinned() const
{
    return bCPUSkinned;
}

FString USkeletalMesh::GetObjectName() const
{
    return RenderData.ObjectName;
}

void USkeletalMesh::SetData(FSkeletalMeshRenderData InRenderData,
    FReferenceSkeleton InRefSkeleton, 
    TArray<FMatrix> InInverseBindPoseMatrices, 
    TArray<UMaterial*> InMaterials)
{
    RenderData = InRenderData;
    RefSkeleton = InRefSkeleton;
    InverseBindPoseMatrices = InInverseBindPoseMatrices;
    Materials = InMaterials;
    //for (const FSkelMeshRenderSection& Section : RenderData->RenderSections)
    //{
    //    DuplicateVerticesSection NewSection;
    //    NewSection.Vertices = Section.Vertices;
    //    DuplicatedVertices.Add(NewSection);
    //}
}
