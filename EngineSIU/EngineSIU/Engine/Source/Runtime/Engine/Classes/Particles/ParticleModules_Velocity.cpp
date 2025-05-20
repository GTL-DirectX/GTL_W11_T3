#include "ParticleModuleVelocity.h"

#include "ParticleEmitterInstances.h"
#include "ParticleSystemComponent.h"
#include "Distributions/DistributionFloatUniform.h"
#include "Distributions/DistributionVectorUniform.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"


void UParticleModuleVelocity::PostInitProperties()
{
    StartVelocity.Distribution = FObjectFactory::ConstructObject<UDistributionVectorUniform>(this);
    if (auto* Dist = Cast<UDistributionVectorUniform>(StartVelocity.Distribution))
    {
        Dist->Min = FVector(-1.f, -1.f, -1.f);
        Dist->Max = FVector( 1.f,  1.f, 1.f);
    }

    StartVelocityRadial.Distribution = FObjectFactory::ConstructObject<UDistributionFloatUniform>(this);
    if (auto* Radial = Cast<UDistributionFloatUniform>(StartVelocityRadial.Distribution))
    {
        Radial->Min = 0.f;
        Radial->Max = 5.f;
    }
}

void UParticleModuleVelocity::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    FBaseParticle& Particle = *ParticleBase;
    const float Time = Owner->EmitterTime;
    
    // 1. 기본 속도 분포 평가
    FVector LinearVelocity = StartVelocity.GetValue(Time, Owner->Component, 0);  // Offset은 보통 페이로드 offset, 여기선 0

    // 2. 공간 변환: Local → Simulation
    LinearVelocity = Owner->EmitterToSimulation.TransformVector(LinearVelocity);

    // 3. 방사형 속도 값 평가
    float RadialSpeed = StartVelocityRadial.GetValue(Time);

    // 4. Emitter 중심 방향 단위 벡터
    FVector FromOrigin = (Particle.Location - Owner->EmitterToSimulation.GetOrigin()).GetSafeNormal();
    FVector RadialVelocity = FromOrigin * RadialSpeed;


    // 5. 최종 속도 = 기본 + 방사형
    FVector FinalVelocity = LinearVelocity + RadialVelocity;

    // 6. 파티클에 누적
    Particle.Velocity += (FVector)FinalVelocity;
    Particle.BaseVelocity += (FVector)FinalVelocity;

    UE_LOG(ELogLevel::Error,
        TEXT("[Velocity] OffsetIdx=%d SpawnTime=%.3f FinalVel=(%.1f, %.1f, %.1f)"),
        Offset, SpawnTime,
        FinalVelocity.X, FinalVelocity.Y, FinalVelocity.Z);
}
