#include "ParticleEmitterInstances.h"
#include "ParticleModuleRequired.h"
#include "Engine/FObjLoader.h"
#include "UObject/ObjectFactory.h"

UParticleModuleRequired::UParticleModuleRequired()
{
    bEnabled = true;
    bSpawnModule = true;
    bUpdateModule = false;
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
