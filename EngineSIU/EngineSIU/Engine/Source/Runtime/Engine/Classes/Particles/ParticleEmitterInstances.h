#pragma once

#include "HAL/PlatformType.h"
#include "ParticleHelper.h"

class UParticleEmitter;
class UParticleSystemComponent;
class UParticleLODLevel;

struct FParticleEmitterInstance
{
    UParticleEmitter* SpriteTemplate;

    // Owner
    UParticleSystemComponent* Component;

    int32 CurrentLODLevelIndex;
    UParticleLODLevel* CurrentLODLevel;

    /* Emitter instance의 위치 / Emitter 로컬 ->Simulation 기준 공간 변환 / */
    FVector Location;
    FMatrix EmitterToSimulation;
    FMatrix SimulationToWorld;

    // Collision Enable? // 추가구현 항목이지만 우선 추가.
    bool bEnableCollision;

    /** (파티클 데이터 / 인덱스 / 인스턴스 데이터 / 인스턴스 데이터 크기) 배열  포인터 */
    uint8* ParticleData;
    uint16* ParticleIndices;
    uint8* InstanceData;
    int32 InstancePayloadSize;

    /** 파티클 데이터의 오프셋 / 파티클 총 크기 / ParticleData 배열 내부 Stride */
    int32 PayloadOffset;
    int32 ParticleSize;
    int32 ParticleStride;

    /** Emitter 내부 현재 활성화된 Particle 개수 / 단조 증가 카운터 */
    int32 ActiveParticles;
    uint32 ParticleCounter;

    /** 파티클 배열 내 최대 활성화 개수 **/
    int32 MaxActiveParticles;

    /* ex) 지난 프레임에서 생성해야 했던 파티클의 소수점 개수*/
    float SpawnFraction; 
    float EmitterTime;
    float LastDeltaTime;

    /* 컴포넌트가 이 Emitter 비활성화시킬 수 있음 */
    uint8 bEnabled : 1;

    FParticleEmitterInstance();

    // 각 Emitter Type에 맞게 오버라이딩 필요
    virtual FDynamicEmitterDataBase* GetDynamicData(bool bSelected);
    virtual bool FillReplayData(FDynamicEmitterReplayDataBase& OutData);


    virtual void Tick(float DeltaTime, bool bSuppressSpawning);
    virtual float Tick_EmitterTimeSetup(float DeltaTime, UParticleLODLevel* InCurrentLODLevel);
    virtual void Tick_ModuleUpdate(float DeltaTime, UParticleLODLevel* InCurrentLODLevel);
    virtual float Tick_SpawnParticles(float DeltaTime, UParticleLODLevel* InCurrentLODLevel, bool bSuppressSpawning, bool bFirstTime);


    virtual float Spawn(float DeltaTime);
    virtual void PreSpawn(FBaseParticle* Particle, const FVector& InitialLocation, const FVector& InitialVelocity);
    virtual void PostSpawn(FBaseParticle* Particle, float InterpolationPercentage, float SpawnTime);

    void SpawnParticles(int32 Count, float StartTime, float Increment, const FVector& InitialLocation, const FVector& InitialVelocity, struct FParticleEventInstancePayload* EventPayload);

    void KillParticles();
    void KillParticle(int32 Index);

    virtual void OnEmitterInstanceKilled(FParticleEmitterInstance* Instance);

    uint32 GetModuleDataOffset(UParticleModule* Module) const;
    uint8* GetModuleInstanceData(UParticleModule* Module) const;

    virtual void Init(UParticleSystemComponent* InComponent, int32 InEmitterIndex);
    void UpdateTransforms();
    virtual bool Resize(int32 NewMaxActiveParticles, bool bSetMaxActiveCount = true);
    virtual void Rewind();
    virtual void UpdateBoundingBox(float DeltaTime);
};
