#include "ParticleModuleLifeTime.h"

#include "UObject/Casts.h"
#include "ParticleEmitterInstances.h"

void UParticleModuleLifeTime::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    SPAWN_INIT;
    float MaxLifeTime = LifeTime.GetValue(Owner->EmitterTime, Cast<UObject>(Owner->Component));
    
    if (Particle.OneOverMaxLifetime > 0.0f)
    {
        // 다른 모듈이 이미 LifeTime을 설정한 경우, MaxLifeTime을 조정하여 OneOverMaxLifetime을 유지.
        Particle.OneOverMaxLifetime = 1.0f / (MaxLifeTime + 1.f / Particle.OneOverMaxLifetime);
    }
    else
    {
        // 다른 모듈이 관여하지 않은 경우.
        Particle.OneOverMaxLifetime = MaxLifeTime > 0.0f ? 1.0f / MaxLifeTime : 0.0f;
    }

    Particle.RelativeTime = Particle.RelativeTime > 1.0f ? Particle.RelativeTime : SpawnTime * Particle.OneOverMaxLifetime;
}
