#include "ParticleModuleTypeDataMesh.h"

#include "Components/Mesh/StaticMeshRenderData.h"
#include "Engine/FObjLoader.h"
#include "UObject/Casts.h"

void UParticleModuleTypeDataMesh::PostInitProperties()
{
    Super::PostInitProperties();
    Mesh = FObjManager::GetStaticMesh(L"Contents/Reference/Reference.obj");
}

UObject* UParticleModuleTypeDataMesh::Duplicate(UObject* InOuter)
{
    UParticleModuleTypeDataMesh* NewObject = Cast<UParticleModuleTypeDataMesh>(Super::Duplicate(InOuter));
    if (NewObject)
    {
        NewObject->Mesh = Mesh;
        // NewObject->Mesh = Cast<UStaticMesh>(Mesh->Duplicate(InOuter)); 
    }
    return NewObject;
}
