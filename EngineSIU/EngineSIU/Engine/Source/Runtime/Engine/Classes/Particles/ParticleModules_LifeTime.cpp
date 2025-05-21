#include "ParticleModuleLifeTime.h"
#include "UObject/Casts.h"
#include "ParticleEmitterInstances.h"
#include "Distributions/DistributionFloatUniform.h"
#include "UObject/ObjectFactory.h"


UParticleModuleLifeTime::UParticleModuleLifeTime()
{
    bEnabled = true;
    bSpawnModule = true;
    bUpdateModule = false;
}

UObject* UParticleModuleLifeTime::Duplicate(UObject* InOuter)
{
    UParticleModuleLifeTime* NewModule = Cast<UParticleModuleLifeTime>(Super::Duplicate(InOuter));
    if (NewModule)
    {
        NewModule->LifeTime = LifeTime;
    }
    return NewModule;
}

/* LifeTime 값 초기화 */
void UParticleModuleLifeTime::PostInitProperties()
{
    Super::PostInitProperties();
    LifeTime.Distribution = FObjectFactory::ConstructObject<UDistributionFloatUniform>(this);
    if (auto* Dist = Cast<UDistributionFloatUniform>(LifeTime.Distribution))
    {
        Dist->Min = 3.f;
        Dist->Max = 10.f;
    }
}

void UParticleModuleLifeTime::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    SPAWN_INIT;
    float MaxLifeTime = LifeTime.GetValue(Owner->EmitterTime, Cast<UObject>(Owner->Component));

    if (Particle.OneOverMaxLifetime > 0.0f)
    {
        // 다른 모듈이 이미 LifeTime을 설정한 경우, MaxLifeTime을 조정하여 OneOverMaxLifetime을 유지.
        // 두 수명을 병합 : 더 작은 수명을 우선시하며 혼합
        Particle.OneOverMaxLifetime = 1.0f / (MaxLifeTime + 1.f / Particle.OneOverMaxLifetime);
    }
    else
    {
        // 다른 모듈이 관여하지 않은 경우.
        Particle.OneOverMaxLifetime = MaxLifeTime > 0.0f ? 1.0f / MaxLifeTime : 0.0f;
    }

    Particle.RelativeTime = Particle.RelativeTime > 1.0f ? Particle.RelativeTime : SpawnTime * Particle.OneOverMaxLifetime;

}
