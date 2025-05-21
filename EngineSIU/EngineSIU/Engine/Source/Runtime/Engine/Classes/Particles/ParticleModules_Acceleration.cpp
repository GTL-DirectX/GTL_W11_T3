#include "ParticleEmitterInstances.h"
#include "ParticleModuleAcceleration.h"
#include "ParticleSystemComponent.h"
#include "Distributions/DistributionVectorUniform.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"


UParticleModuleAcceleration::UParticleModuleAcceleration()
{
    bEnabled = true;
    bSpawnModule = true;
    bUpdateModule = true;
}

void UParticleModuleAcceleration::PostInitProperties()
{
    Super::PostInitProperties();
    GravityAccel.Distribution = FObjectFactory::ConstructObject<UDistributionVectorUniform>(this);
    if (auto* Dist = Cast<UDistributionVectorUniform>(GravityAccel.Distribution))
    {
        Dist->Min = FVector(0.f, 0.f, -9.8f);
        Dist->Max = FVector(0.f, 0.f, -9.8f);
    }
}

UObject* UParticleModuleAcceleration::Duplicate(UObject* InOuter)
{
    UParticleModuleAcceleration* NewModule = Cast<UParticleModuleAcceleration>(Super::Duplicate(InOuter));
    if (NewModule)
    {
        NewModule->GravityAccel = GravityAccel;
    }
    return NewModule;
}

void UParticleModuleAcceleration::Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime)
{
    if (!bEnabled || !Owner || Owner->ActiveParticles <= 0)
    {
        return;
    }

    const int32   ActiveParticles = Owner->ActiveParticles;
    const uint32  ParticleStride = Owner->ParticleStride;
    uint16* ParticleIndices = Owner->ParticleIndices;
    uint8* ParticleData = Owner->ParticleData;

    BEGIN_MY_UPDATE_LOOP
        // RelativeTime (0.0 ~ 1.0)에 따라 가속도 값 평가
        FVector Gravity = GravityAccel.GetValue(Particle.RelativeTime, Owner->Component);

        // 속도에 중력 적용
        Particle.Velocity += Gravity * DeltaTime;
    END_MY_UPDATE_LOOP
}

