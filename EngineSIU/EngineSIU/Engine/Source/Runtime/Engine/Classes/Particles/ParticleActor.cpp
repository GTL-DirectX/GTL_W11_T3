#include "ParticleActor.h"

#include "ParticleModuleRequired.h"
#include "ParticleSystemComponent.h"
#include "ParticleSystem.h"
#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"
#include "ParticleModuleLifeTime.h"
#include "ParticleModuleLocation.h"
#include "ParticleModuleVelocity.h"
#include "TypeData/ParticleModuleTypeDataSprite.h"
#include "TypeData/ParticleModuleTypeDataMesh.h"
#include "Components/Mesh/StaticMeshRenderData.h"
#include "Engine/AssetManager.h"
#include "Engine/FObjLoader.h"

void AParticleActor::PostSpawnInitialize()
{
    const TMap<FName, FAssetInfo> Assets = UAssetManager::Get().GetAssetRegistry();
    Super::PostSpawnInitialize();

    ParticleSystemComponent = AddComponent<UParticleSystemComponent>(FName("ParticleSystemComponent_0"));
    RootComponent = ParticleSystemComponent;

    ParticleSystemComponent->InitializeSystem();

    // 하드코딩된 초기 위치/속도 세팅
    ParticleSystemComponent->InitialLocationHardcoded = FVector::ZeroVector;
    ParticleSystemComponent->InitialVelocityHardcoded = FVector(0.f, 0.f, 1.f);
}

UObject* AParticleActor::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewActor->ParticleSystemComponent = Cast<UParticleSystemComponent>(ParticleSystemComponent->Duplicate(InOuter));

    return NewActor;
}

void AParticleActor::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(ELogLevel::Error, TEXT("AParticleActor::BeginPlay - Simulation started"));
}
