#include "ParticleEmitter.h"

#include "ParticleHelper.h"
#include "ParticleLODLevel.h"
#include "ParticleModule.h"

UParticleLODLevel* UParticleEmitter::GetLODLevel(int32 LODLevel)
{
    if (LODLevel >= LODLevels.Num())
    {
        return nullptr;
    }

    return LODLevels[LODLevel];
}

void UParticleEmitter::Build()
{
    /* 최소 1개의 LODLevel이 있을 때에만 파티클 사이즈 / 오프셋 데이터 설정*/
    const int32 LODCount = LODLevels.Num();
    if (LODCount > 0)
    {
        /* Cache particle size/offset data for all LOD Levels */
        CacheEmitterModuleInfo();
    }
}

/*
 * Per-Particle (고정) 데이터와 Per-Instance (가변) 데이터를 모두 누적하여 수정
 * [현재] : TypeDataModule 계열이 없으므로, 일반 모듈만 고려
 */
void UParticleEmitter::CacheEmitterModuleInfo()
{
    ModuleOffsetMap.Empty();
    ModuleInstanceOffsetMap.Empty();
    ModulesNeedingInstanceData.Empty();

    ParticleSize = sizeof(FBaseParticle);
    ReqInstanceBytes = 0;

    if (LODLevels.Num() == 0)
    {
        return;
    }

    UParticleLODLevel* HighLODLevel = GetLODLevel(0);
    // High LOD Level 의 모듈 순회
    for (int32 ModuleIdx = 0; ModuleIdx < HighLODLevel->Modules.Num(); ++ModuleIdx)
    {
        UParticleModule* ParticleModule = HighLODLevel->Modules[ModuleIdx];

        // 1) Per-Particle 고정 데이터 바이트 누적
        int32 FixedBytes = static_cast<int32>(ParticleModule->RequiredBytes(nullptr));
        if (FixedBytes > 0)
        {
            ModuleOffsetMap.Add(ParticleModule, ParticleSize);
            ParticleSize += FixedBytes;
        }

        // 2) Per-Instance 데이터 바이트 누적
        int32 InstanceBytes = static_cast<int32>(ParticleModule->RequiredBytesPerInstance());
        if (InstanceBytes > 0)
        {
            // 이 에미터 인스턴스 블록에서의 오프셋 저장
            ModuleInstanceOffsetMap.Add(ParticleModule, ReqInstanceBytes);
            ModulesNeedingInstanceData.Add(ParticleModule);

            // 다른 LOD 레벨의 동일 모듈에도 같은 오프셋 매핑
            for (int32 LODIdx = 1; LODIdx < LODLevels.Num(); ++LODIdx)
            {
                UParticleModule* LODModule = LODLevels[LODIdx]->Modules[ModuleIdx];
                ModuleInstanceOffsetMap.Add(LODModule, ReqInstanceBytes);
            }

            // 다음 모듈을 위한 오프셋 증가
            ReqInstanceBytes += InstanceBytes;
        }
    }

    // 이제 ParticleSize 는 모든 모듈의 고정 데이터 크기를,
    // ReqInstanceBytes 는 모든 모듈의 인스턴스 데이터 크기를 합산한 값
}

void UParticleEmitter::PostLoad()
{
    
}

UParticleLODLevel* UParticleEmitter::GetCurrentLODLevel(FParticleEmitterInstance* Instance) const
{
    return Instance->CurrentLODLevel;
}
