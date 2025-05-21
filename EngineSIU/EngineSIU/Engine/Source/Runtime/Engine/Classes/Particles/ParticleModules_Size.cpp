#include "ParticleModuleSize.h"
#include "UObject/Casts.h"
#include "ParticleEmitterInstances.h"
#include "Distributions/DistributionVectorUniform.h"
#include "UObject/ObjectFactory.h"

UParticleModuleSize::UParticleModuleSize()
{
    bEnabled = true;
    bSpawnModule = true;
    bUpdateModule = true;
}

UObject* UParticleModuleSize::Duplicate(UObject* InOuter)
{
    UParticleModuleSize* NewModule = Cast<UParticleModuleSize>(Super::Duplicate(InOuter));
    if (NewModule)
    {
        NewModule->StartSize = StartSize;
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


void UParticleModuleSize::Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime)
{
    if (!bEnabled || !Owner)
    {
        return;
    }

    const int32   ActiveParticles = Owner->ActiveParticles;
    const uint32  ParticleStride = Owner->ParticleStride;
    uint16* ParticleIndices = Owner->ParticleIndices;
    uint8* ParticleData = Owner->ParticleData;

    // color-over-life 적용
    BEGIN_MY_UPDATE_LOOP
        float t = Particle.RelativeTime;
        FVector initSize = Particle.BaseSize;
        Particle.Size = FMath::Lerp(initSize+2.0f, FVector::ZeroVector, t);
    END_MY_UPDATE_LOOP
}
