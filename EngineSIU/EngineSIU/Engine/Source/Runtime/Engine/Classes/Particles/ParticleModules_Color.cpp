#include "ParticleModuleColor.h"

#include "UObject/Casts.h"
#include "ParticleEmitterInstances.h"
#include "Distributions/DistributionFloatUniform.h"
#include "UObject/ObjectFactory.h"


UParticleModuleColor::UParticleModuleColor()
{
    bEnabled = true;
    bSpawnModule = true;
    bUpdateModule = true;
}

UObject* UParticleModuleColor::Duplicate(UObject* InOuter)
{
    UParticleModuleColor* NewModule = Cast<UParticleModuleColor>(Super::Duplicate(InOuter));
    if (NewModule)
    {
        NewModule->Color = Color;
    }
    return NewModule;
}

/* LifeTime 값 초기화 */
void UParticleModuleColor::PostInitProperties()
{
    Super::PostInitProperties();
    Color = FLinearColor::Red;
}

void UParticleModuleColor::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    SPAWN_INIT;
    Particle.Color = Color;
}

void UParticleModuleColor::Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime)
{
    if (!bEnabled || !Owner)
    {
        return;
    }

    // 로컬 변수 설정
    const int32   ActiveParticles = Owner->ActiveParticles;
    const uint32  ParticleStride = Owner->ParticleStride;
    uint16* ParticleIndices = Owner->ParticleIndices;
    uint8* ParticleData = Owner->ParticleData;

    // color-over-life 적용
    BEGIN_MY_UPDATE_LOOP
        // RelativeTime 에 따라 white -> 모듈의 Color 로 선형 보간
        float t = Particle.RelativeTime;
        Particle.Color = FMath::Lerp(FLinearColor::White, Color, t);
    END_MY_UPDATE_LOOP
}


