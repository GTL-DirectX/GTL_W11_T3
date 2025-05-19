#include "ParticleModuleVelocity.h"

#include "ParticleEmitterInstances.h"
#include "ParticleSystemComponent.h"

void UParticleModuleVelocity::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    //Super::Spawn(Owner, Offset, SpawnTime, ParticleBase);

    FBaseParticle& Particle = *ParticleBase;

    // TODO : StartVelocity 분포에서 속도값 평가 로직 GetValue 추가하기
    FVector Vel = StartVelocity;

    // 공간 변환 (Local → Simulation)
    Vel = Owner->EmitterToSimulation.TransformVector(Vel);
    //FVector Radial = FromOrigin * StartVelocityRadial.GetValue(...);

    // TODO : Radial 방향 속도 추가 (Distribution 반영)



    Particle.Velocity += Vel;
    Particle.BaseVelocity += Vel;
}
