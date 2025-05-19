#pragma once

#include "ParticleEmitterInstances.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Container/Array.h"

class UParticleLODLevel;

enum class EEmitterRenderMode : int
{
    ERM_Normal,
    ERM_Point,
    ERM_Cross,
    ERM_LightsOnly,
    ERM_None,
    ERM_MAX,
};


class UParticleEmitter : public UObject
{
    DECLARE_CLASS(UParticleEmitter, UObject)

public:
    UParticleEmitter() = default;
    void Build();
    void CacheEmitterModuleInfo();
    void PostLoad();


    UParticleLODLevel* GetCurrentLODLevel(FParticleEmitterInstance* Instance) const;
    UParticleLODLevel* GetLODLevel(int32 LODLevel);

    FName EmitterName;
    int32 ParticleSize;
    int32 InitialAllocationCount;
    int32 ReqInstanceBytes;


    TArray<UParticleLODLevel*> LODLevels;

    /* per-instance 데이터를 정적으로 보관하거나 초기화가 필요한 모듈들 목록
     * 각 모듈은 PrepPerInstanceBlock()을 갖고 있으며, 여기에 필요 정보들을 설정함
     */

    /* 파티클 시스템이 실행되는 동안, 모듈마다 per-instance 데이터가 필요한 경우
     * RequiredBytesPerInstance > 0 인 모듈들
     */
    TArray<UParticleModule*> ModulesNeedingInstanceData;

    /* 각 모듈이 파티클당 사용하는 per-particle payload(offset) 를 관리하는 맵 */
    TMap<UParticleModule*, uint32> ModuleOffsetMap;

    /* 이 모듈이 사용할 고유 데이터를 담고 있는 버퍼(InstanceData) 내 offset */
    TMap<UParticleModule*, uint32> ModuleInstanceOffsetMap;

};
