#include "ParticleEmitterInstances.h"
#include "ParticleModuleRequired.h"
#include "Engine/FObjLoader.h"
#include "UObject/ObjectFactory.h"

#include "UObject/Casts.h"

UParticleModuleRequired::UParticleModuleRequired()
{
    bEnabled = true;
    bSpawnModule = true;
    bUpdateModule = false;
}

UObject* UParticleModuleRequired::Duplicate(UObject* InOuter)
{
    UParticleModuleRequired* NewModule = Cast<UParticleModuleRequired>(Super::Duplicate(InOuter));
    if (NewModule)
    {
        NewModule->Material = Material;
        NewModule->EmitterOrigin = EmitterOrigin;
        NewModule->EmitterRotation = EmitterRotation;
        NewModule->SortMode = SortMode;
        NewModule->bUseLocalSpace = bUseLocalSpace;
        NewModule->bKillOnDeactivate = bKillOnDeactivate;
        NewModule->bKillOnCompleted = bKillOnCompleted;
        NewModule->SpawnRate = SpawnRate;
        NewModule->EmitterDuration = EmitterDuration;
    }
    return NewModule;
    
}

void UParticleModuleRequired::PostInitProperties()
{
    Super::PostInitProperties();
    
    EmitterDuration = 3.0f;
    SpawnRate = 10.0f / EmitterDuration;
    Material = FObjManager::GetMaterial("Star");
}

void UParticleModuleRequired::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    Super::Spawn(Owner, Offset, SpawnTime, ParticleBase);
    
    // TODO : Material Setting 방식 더 나은 방식 생각해봐야함
    if (!Owner->Material)
        Owner->Material = Material;
}
