#include "ParticleEmitterInstances.h"
#include "ParticleModuleLocation.h"
#include "Distributions/DistributionVectorUniform.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"


UParticleModuleLocation::UParticleModuleLocation()
{
    bEnabled = true;
    bSpawnModule = true;
    bUpdateModule = false;
}

UObject* UParticleModuleLocation::Duplicate(UObject* InOuter)
{
    UParticleModuleLocation* NewModule = Cast<UParticleModuleLocation>(Super::Duplicate(InOuter));
    if (NewModule)
    {
        NewModule->StartLocation = StartLocation;
    }
    return NewModule;
}

void UParticleModuleLocation::PostInitProperties()
{
    Super::PostInitProperties();
    StartLocation.Distribution = FObjectFactory::ConstructObject<UDistributionVectorUniform>(this);
    if (auto* Dist = Cast<UDistributionVectorUniform>(StartLocation.Distribution))
    {
        Dist->Min = FVector(-1.f, -1.f, -1.f);
        Dist->Max = FVector(1.f, 1.f, 1.f);
    }
}

void UParticleModuleLocation::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    FBaseParticle& Particle = *ParticleBase;

    // 1) 로컬 오프셋 (ImGui에서 설정한 StartLocation) 가져오기
    FVector LocationOffset = StartLocation.GetValue(Owner->EmitterTime, Cast<UObject>(Owner->Component), 0);;

    // 2) 월드 -> 시뮬레이션 공간 변환 (EmitterToSimulation은 컴포넌트의 transform)
    LocationOffset = Owner->EmitterToSimulation.TransformPosition(LocationOffset);

    Particle.Location += LocationOffset;
    Particle.OldLocation += LocationOffset;

    UE_LOG(ELogLevel::Error,
        TEXT("[Location] OffsetIdx=%d SpawnTime=%.3f NewLoc=(%.1f,%.1f,%.1f)"),
        Offset, SpawnTime,
        Particle.Location.X, Particle.Location.Y, Particle.Location.Z);
}
