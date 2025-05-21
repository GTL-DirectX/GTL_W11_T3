#include "ParticleModuleSize.h"
#include "UObject/Casts.h"
#include "ParticleEmitterInstances.h"
#include "Distributions/DistributionVectorUniform.h"
#include "UObject/ObjectFactory.h"

UParticleModuleSize::UParticleModuleSize()
{
    bEnabled = true;
    bSpawnModule = true;
    bUpdateModule = false;
}

UObject* UParticleModuleSize::Duplicate(UObject* InOuter)
{
    UParticleModuleSize* NewModule = Cast<UParticleModuleSize>(Super::Duplicate(InOuter));
    if (NewModule)
    {
        NewModule->StartSize = StartSize;
        NewModule->StartSize.Distribution = Cast<UDistributionVectorUniform>(StartSize.Distribution->Duplicate(InOuter));
    }
    return NewModule;
}


void UParticleModuleSize::PostInitProperties()
{
    Super::PostInitProperties();
    StartSize.Distribution = FObjectFactory::ConstructObject<UDistributionVectorUniform>(this);
    if (auto* Dist = Cast<UDistributionVectorUniform>(StartSize.Distribution))
    {
        Dist->Min = FVector(1.f, 1.f, 1.f);
        Dist->Max = FVector(1.f, 1.f, 1.f);
    }
}

void UParticleModuleSize::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    SPAWN_INIT
    FVector Size = StartSize.GetValue(Owner->EmitterTime, Cast<UObject>(Owner->Component), 0);
    Particle.Size += Size;
    Particle.BaseSize += Size;
}


