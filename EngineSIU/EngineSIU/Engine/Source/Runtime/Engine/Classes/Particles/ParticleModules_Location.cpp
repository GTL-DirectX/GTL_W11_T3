#include "ParticleEmitterInstances.h"
#include "ParticleModuleLocation.h"


void UParticleModuleLocation::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    FBaseParticle& Particle = *ParticleBase;

    // 1) 로컬 오프셋 (ImGui에서 설정한 StartLocation) 가져오기
    FVector LocationOffset = StartLocation;

    // 2) 월드 -> 시뮬레이션 공간 변환 (EmitterToSimulation은 컴포넌트의 transform)
    LocationOffset = Owner->EmitterToSimulation.TransformPosition(LocationOffset);

    Particle.Location += LocationOffset;
    Particle.OldLocation += LocationOffset;
}
