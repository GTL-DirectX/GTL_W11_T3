#include "ParticleEmitterInstances.h"


/*
 * 수명 다한 파티클 제거. (Active array로부터 제거)
 */
void FParticleEmitterInstance::KillParticles()
{
}

void FParticleEmitterInstance::KillParticle(int32 Index)
{
    
}




void FParticleEmitterInstance::Tick(float DeltaTime, bool bSuppressSpawning)
{
    UParticleLODLevel* LODLevel = /*GetCurrentLODLevelChecked()*/ nullptr;
    float EmitterDelay = Tick_EmitterTimeSetup(DeltaTime, LODLevel);

    if (bEnabled)
    {
        // 수명이 다했거나 RelativeTime > 1.0 인 파티클을 제거.
        KillParticles();
        //Tick_ModuleUpdate(DeltaTime, LODLevel);

        //SpawnFraction = Tick_SpawnParticles(DeltaTime, LODLevel, bSuppressSpawning, bFirstTime);

        // beams의 경우 postupdate 추가 필요
        if (ActiveParticles > 0)
        {
            // Orbit Module 존재시 UpdateOrbitData 추가 필요
            //UpdateBoundingBox(DeltaTime);
        }
    }

    EmitterTime += EmitterDelay;
}

// 파티클 시스템 전체의 진행 시간 증가 및 딜레이, 루프, 종료 조건 처리
// @return : 실제 시뮬레이션에 쓰이지 않은 시간(딜레이)
float FParticleEmitterInstance::Tick_EmitterTimeSetup(float DeltaTime, UParticleLODLevel* InCurrentLODLevel)
{
}

void FParticleEmitterInstance::Tick_ModuleUpdate(float DeltaTime, UParticleLODLevel* InCurrentLODLevel)
{
    //UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
    //for (int32 ModuleIndex = 0; ModuleIndex < InCurrentLODLevel->Modules.Num(); ModuleIndex++)
    //{
    //    UParticleModule* CurrentModule = InCurrentLODLevel->UpdateModules[ModuleIndex];
    //    if (CurrentModule && CurrentModule->bEnabled && CurrentModule->bUpdateModule)
    //    {
    //        CurrentModule->Update(this, GetModuleDataOffset(HighestLODLevel->UpdateModules[ModuleIndex]), DeltaTime);
    //    }
    //}
}

float FParticleEmitterInstance::Tick_SpawnParticles(float DeltaTime, UParticleLODLevel* InCurrentLODLevel, bool bSuppressSpawning, bool bFirstTime)
{
    // [간략] significance 중지 또는 외부 중단 명령 고려 안함
    if (/*!bHaltSpawning && !bHaltSpawningExternal && */!bSuppressSpawning && (EmitterTime >= 0.0f))
    {
        // 스폰 시도 조건 : Emitter가 루프 중이거나 처음 생성된 경우
        // 무한 반복 || 아직 루프 수 채우지 않음 || 시간이 아직 남음 || 생성 이후 첫 프레임
        //if ((EmitterLoops == 0) ||
        //    (LoopCount < EmitterLoops) ||
        //    (SecondsSinceCreation < (EmitterDuration * EmitterLoops)) ||
        //    bFirstTime)
        //{
        //    bFirstTime = false;
        //    SpawnFraction = Spawn(DeltaTime);
        //}
    }
    return SpawnFraction;
}

/*
* 우선 Burst고려 하지 않고 간략화함
* 필수 기능 :
* 1. SpawnRate 기반 파티클 생성 수 계산
* 2. SpawnFraction 누적 계산
* 3. DeltaTime 기반 스폰 수 계산
* 4. SpawnParticles()로 실제 생성
*/
float FParticleEmitterInstance::Spawn(float DeltaTime)
{
    // SpawnRate 기반 파티클 생성 수 계산
    // 생성할 파티클 수 결정 시, SpawnParticles()로 실제 생성 (일반용, Burst용)
    // 누적되지 못한 잔여 시간(SpawnFraction) 다음 프레임으로 넘김

    float SpawnRate = 0.0f;            // 초당 생성할 파티클 수
    int32 SpawnCount = 0;              // 이번 프레임에 생성할 일반 파티클 수
    float OldLeftover = SpawnFraction; // 지난 프레임에 못 만든 누적된 스폰 타이밍

    // 간략화 버전
    // 잔여 분수 관리 (OldLeftOver, NewLeftOver)
    // 정수 파티클 개수 분리 (StartTime, Increment 계산 등)
    // 풀 크기 클램핑 (Resize)
    // 최대치 검사 (미리 할당된 파티클 풀 크기 내에서만 생성되도록 생성개수 클램핑)
    if ((SpawnRate > 0.f) /*|| (BurstCount > 0)*/)
    {
        //SpawnParticles(Number, StartTime, Increment, InitialLocation, FVector::ZeroVector, EventPayload);

    }

    /*
      float TotalSpawn = SpawnRate * DeltaTime + SpawnFraction;
      int SpawnCount = (int)TotalSpawn;
      SpawnFraction = TotalSpawn - SpawnCount;
      
      for (int i = 0; i < SpawnCount; ++i)
          SpawnParticle();
     */
    return SpawnFraction;
}

void FParticleEmitterInstance::SpawnParticles(int32 Count, float StartTime, float Increment, const FVector& InitialLocation, const FVector& InitialVelocity, FParticleEventInstancePayload* EventPayload)
{
    for (int32 i = 0; i < Count; i++)
    {
        // Macro 추가 필요.
        //DECLARE_PARTICLE_PTR
        //PreSpawn(Particle, InitialLocation, InitialVelocity);

        //for (int32 ModuleIndex = 0; ModuleIndex < LODLevel->SpawnModules.Num(); ModuleIndex++)
        //{
        //    ...
        //}

        //PostSpawn(Particle, Interp, SpawnTime);
    }
}


void FParticleEmitterInstance::OnEmitterInstanceKilled(FParticleEmitterInstance* Instance)
{
    /*if (SourceEmitter == Instance)
    {
        SourceEmitter = NULL;
    }
    if (TargetEmitter == Instance)
    {
        TargetEmitter = NULL;
    }*/

}

void FParticleEmitterInstance::Rewind()
{
    // SecondsSinceCreation = 0;
    // EmitterTime = 0;
    // LoopCount = 0;
    ParticleCounter = 0;
    // bEnabled = 1;
}
