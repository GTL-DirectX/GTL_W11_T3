#include "ParticleEmitterInstances.h"

#include <algorithm>

#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"
#include "ParticleModuleRequired.h"
#include "TypeData/ParticleModuleTypeDataBase.h"
#include "ParticleSystem.h"
#include "ParticleSystemComponent.h"
#include "Engine/Asset/StaticMeshAsset.h"
#include "Templates/AlignmentTemplates.h"
#include "UObject/ObjectFactory.h"
#include "TypeData/ParticleModuleTypeDataMesh.h"
#include "UObject/Casts.h"

void FParticleDataContainer::Alloc(int32 InParticleDataNumBytes, int32 InParticleIndicesNumShorts)
{
    if (InParticleDataNumBytes > 0 && ParticleIndicesNumShorts >= 0
        && InParticleDataNumBytes % sizeof(uint16) == 0)
    {
        assert(InParticleDataNumBytes > 0 && ParticleIndicesNumShorts >= 0
            && InParticleDataNumBytes % sizeof(uint16) == 0); // we assume that the particle storage has reasonable alignment below);
    }

    ParticleDataNumBytes = InParticleDataNumBytes;
    ParticleIndicesNumShorts = InParticleIndicesNumShorts;

    MemBlockSize = ParticleDataNumBytes + ParticleIndicesNumShorts * sizeof(uint16);

    //ParticleData = (uint8*)FastParticleSmallBlockAlloc(MemBlockSize); // 메모리 할당 로직 정의 필요.
    ParticleData = static_cast<uint8*>(FPlatformMemory::Malloc<EAT_Object>(MemBlockSize));
    ParticleIndices = (uint16*)(ParticleData + ParticleDataNumBytes);
}

void FParticleDataContainer::Free()
{
    if (ParticleData)
    {
        if (MemBlockSize > 0)
        {
            assert(MemBlockSize > 0);
        }
        
        //FastParticleSmallBlockFree(ParticleData, MemBlockSize); Free 해주는 로직.
    }
    MemBlockSize = 0;
    ParticleDataNumBytes = 0;
    ParticleIndicesNumShorts = 0;
    ParticleData = nullptr;
    ParticleIndices = nullptr;
}


/*
 * 수명 다한 파티클 제거. (Active array로부터 제거)
 * 뒤에서부터 검사해야 스왑해서 줄인 인덱스가 아직 남은 루프에 영향을 주지 않음
 */
void FParticleEmitterInstance::KillParticles()
{
    int32 i = ActiveParticles;
    // 뒤에서부터 순회해서 swap & pop
    while (i-- > 0)
    {
        int32 DataIdx = ParticleIndices[i];
        FBaseParticle* P = (FBaseParticle*)(ParticleData + DataIdx * ParticleStride);
        if (P->RelativeTime >= 1.0f)
        {
            //UE_LOG(ELogLevel::Error,
            //    TEXT("KillParticles(): Active=%d, removing slot %d"),
            //    ActiveParticles, DataIdx);

            // swap i <-> tail
            /*ParticleIndices[i] = ParticleIndices[ActiveParticles - 1];
            ParticleIndices[ActiveParticles - 1] = DataIdx;
            --ActiveParticles;*/
			std::swap(ParticleIndices[i], ParticleIndices[ActiveParticles - 1]);
            --ActiveParticles;
        }
    }
}

/* 인덱스 풀 방식 구현 : 인덱스만 스왑, 메모리 블록을 Memcpy로 앞당기지 않음 */
void FParticleEmitterInstance::KillParticle(int32 Index)
{
    if (Index >= 0 && Index < ActiveParticles)
    {
        UE_LOG(ELogLevel::Error, TEXT("Active Particles : %d Kill Particle Index = %d"), ActiveParticles, Index);
        ParticleIndices[Index] = ParticleIndices[ActiveParticles - 1];
        ParticleIndices[ActiveParticles - 1] = Index;
        --ActiveParticles;
    }
    
}

FParticleEmitterInstance::FParticleEmitterInstance() :
    SpriteTemplate(NULL)
    , Component(NULL)
    , CurrentLODLevel(NULL)
    , CurrentLODLevelIndex(0)
    , PayloadOffset(0)
    , bEnabled(1)
    , ParticleData(NULL)
    , ParticleIndices(NULL)
    , InstanceData(NULL)
    , InstancePayloadSize(0)
    , ParticleSize(0)
    , ParticleStride(0)
    , ActiveParticles(0)
    , ParticleCounter(0)
    , MaxActiveParticles(20)
    , SpawnFraction(0.0f)
    , EmitterTime(0.0f)
{
}

// Test용으로 우선 Sprite가 기본 Template이라 가정하고 구현, FillReplayData()도 마찬가지
FDynamicEmitterDataBase* FParticleEmitterInstance::GetDynamicData(bool bSelected)
{
    UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
    if (!LODLevel || !bEnabled || !LODLevel->RequiredModule->bEnabled)
    {
        return nullptr;
    }

    FDynamicSpriteEmitterData* NewEmitterData = new FDynamicSpriteEmitterData(LODLevel->RequiredModule);

    if (!FillReplayData(NewEmitterData->Source))
    {
        delete NewEmitterData;
        return nullptr;
    }

    NewEmitterData->bValid = true;
    NewEmitterData->bSelected = bSelected;

    // [로그 출력] : DataContainer 내부 순회하며 파티클 위치/속도 
    {
        uint8*       RawBuffer = NewEmitterData->Source.DataContainer.ParticleData;
        int32        Stride    = NewEmitterData->Source.ParticleStride;
        int32        Count     = NewEmitterData->Source.ActiveParticleCount;

        for (int32 i = 0; i < Count; ++i)
        {
            FBaseParticle* P = reinterpret_cast<FBaseParticle*>(RawBuffer + NewEmitterData->Source.DataContainer.ParticleIndices[i] * Stride);
            /*UE_LOG(ELogLevel::Error,
                   TEXT("GetDynamicData(): Particle[%d] Loc=(%.2f, %.2f, %.2f) Vel=(%.2f, %.2f, %.2f)"),
                   i,
                   P->Location.X, P->Location.Y, P->Location.Z,
                   P->Velocity.X, P->Velocity.Y, P->Velocity.Z);*/
        }
    }
    return NewEmitterData;
}

// 아래 함수도 마찬가지로, Sprite가 기본 Template이라 가정하고 구현
// 차후 SpriteEmitterInstance에서 오버라이딩 필요
bool FParticleEmitterInstance::FillReplayData(FDynamicEmitterReplayDataBase& OutData)
{
    if (ActiveParticles <= 0)
    {
        return false;
    }

    OutData.eEmitterType = DET_Sprite;             // 스프라이트로 기본 가정
    OutData.ActiveParticleCount = ActiveParticles; // 활성 파티클 수
    OutData.ParticleStride = ParticleStride;       // 파티클 하나당 데이터 크기
    OutData.Scale = FVector(1, 1, 1);       // 기본 (1,1,1)
    OutData.SortMode = PSORTMODE_DistanceToView;   // 거리 기준 정렬 예시

    // ParticleStride : 파티클 하나당 바이트 수
    int32 TotalDataBytes = ParticleStride * ActiveParticles;    // 총 바이트 수 
    int32 TotalIndices = ActiveParticles;                       // 인덱스 개수
    OutData.DataContainer.Alloc(TotalDataBytes, TotalIndices);

    int32 Count = ActiveParticles;
    for (uint32 i = 0; i < ActiveParticles; ++i)
    {
        FBaseParticle Particle = *(FBaseParticle*)(ParticleData + ParticleIndices[i] * ParticleStride);
        //UE_LOG(ELogLevel::Error, TEXT("FillReplayData() : Particle[%d] Loc %.2f %.2f %.2f"), i, Particle.Location.X, Particle.Location.Y, Particle.Location.Z);
    }

    // 실제 데이터 복사 : OutData.DataContainer로 파티클 데이터 / 파티클 인덱스 깊은 복사
    /*memcpy(OutData.DataContainer.ParticleData, ParticleData, TotalDataBytes);
    memcpy(OutData.DataContainer.ParticleIndices, ParticleIndices, TotalIndices * sizeof(uint16));*/

    // 1) 각 활성 파티클을 순서대로 복사
    for (int32 i = 0; i < Count; ++i)
    {
        uint16 SrcIdx = ParticleIndices[i];
        // 원본 풀에서 SrcIdx 번 슬롯만 복사
        memcpy(
            OutData.DataContainer.ParticleData + i * ParticleStride,
            ParticleData + SrcIdx * ParticleStride,
            ParticleStride
        );
        // 새 인덱스는 0…Count-1 로 재매핑
        OutData.DataContainer.ParticleIndices[i] = i;
    }

    // Material
    if (OutData.eEmitterType == DET_Sprite)
    {
        auto* SpriteData = dynamic_cast<FDynamicSpriteEmitterReplayDataBase*>(&OutData);
        SpriteData->Material = Material;
    }
    return true;
}

void FParticleEmitterInstance::Tick(float DeltaTime, bool bSuppressSpawning)
{
    //UE_LOG(ELogLevel::Error, "Tick() Start: DeltaTime=%.4f, Active=%d", DeltaTime, ActiveParticles);

    UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
    float EmitterDelay = Tick_EmitterTimeSetup(DeltaTime, LODLevel);

    if (bEnabled)
    {
        bool bFirstTime = (EmitterTime == 0.0f);

        // 수명이 다했거나 RelativeTime > 1.0 인 파티클을 제거.
        // 이유 : Kill before the spawn... Otherwise, we can get 'flashing' (beam2Emitter)
        KillParticles();

        Tick_ModuleUpdate(DeltaTime, LODLevel);

        SpawnFraction = Tick_SpawnParticles(DeltaTime, LODLevel, bSuppressSpawning, bFirstTime);

        // beams의 경우 postupdate 추가 필요
        if (ActiveParticles > 0)
        {
            // Orbit Module 존재시 UpdateOrbitData 추가 필요

            /* 파티클의 실제 Location를 Velocity에 따라 갱신하는 곳 */
            UpdateBoundingBox(DeltaTime);
        }

        EmitterTime += SpawnFraction;
        if (EmitterTime >= CurrentLODLevel->RequiredModule->EmitterDuration)
        {
            EmitterTime = 0.0f;
            bFirstTime = true;
        }
    }

}

// 파티클 시스템 전체의 진행 시간 증가 및 딜레이, 루프, 종료 조건 처리
// @return : 실제 시뮬레이션에 쓰이지 않은 시간(딜레이)
// 현재는 단순화함 : Delay, Looping 무시
float FParticleEmitterInstance::Tick_EmitterTimeSetup(float DeltaTime, UParticleLODLevel* InCurrentLODLevel)
{
    UpdateTransforms();
    return 0.0f;
}

/*
 * 매 프레임마다 파티클의 각 모듈=속성 (위치, 크기, 컬러, 수명 등)을 보간 및 증가
 */
void FParticleEmitterInstance::Tick_ModuleUpdate(float DeltaTime, UParticleLODLevel* InCurrentLODLevel)
{
    // HighLODLevel은 오프셋 계산용
    UParticleLODLevel* HighestLODLevel = SpriteTemplate->LODLevels[0];
    if (HighestLODLevel == nullptr)
    {
        UE_LOG(ELogLevel::Error, "Tick_ModuleUpdate() : LODLevel 0 Not Exists");
        return;
    }

    for (int32 ModuleIndex = 0; ModuleIndex < InCurrentLODLevel->UpdateModules.Num(); ModuleIndex++)
    {
        UParticleModule* Mod = InCurrentLODLevel->UpdateModules[ModuleIndex];
        if (Mod && Mod->bEnabled && Mod->bUpdateModule)
        {
            uint32 Offset = GetModuleDataOffset(HighestLODLevel->UpdateModules[ModuleIndex]);
            Mod->Update(this, Offset, DeltaTime);
        }
    }
}

float FParticleEmitterInstance::Tick_SpawnParticles(float DeltaTime, UParticleLODLevel* InCurrentLODLevel, bool bSuppressSpawning, bool bFirstTime)
{
    // [간략] significance 중지 또는 외부 중단 명령 고려 안함
    if (!bSuppressSpawning && (EmitterTime >= 0.0f) /*!bHaltSpawning && !bHaltSpawningExternal && */)
    {
        // 스폰 시도 조건 : Emitter가 루프 중이거나 처음 생성된 경우
        // 무한 반복 || 아직 루프 수 채우지 않음 || 시간이 아직 남음 || 생성 이후 첫 프레임
        //if (bFirstTime /*(EmitterLoops == 0) || (LoopCount < EmitterLoops)(SecondsSinceCreation < (EmitterDuration * EmitterLoops)) ||*/ )
        //bFirstTime = false;

        SpawnFraction = Spawn(DeltaTime);
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
//float FParticleEmitterInstance::Spawn(float DeltaTime)
//{
//    // SpawnRate 기반 파티클 생성 수 계산
//    // 생성할 파티클 수 결정 시, SpawnParticles()로 실제 생성 (일반용, Burst용)
//    // 누적되지 못한 잔여 시간(SpawnFraction) 다음 프레임으로 넘김
//
//    // --- 1) SpawnRate 가져오기
//    UParticleModuleRequired* Req = CurrentLODLevel->RequiredModule;
//    float Rate = CurrentLODLevel->RequiredModule->SpawnRate; // Distribution 대신 고정 값
//
//    float SpawnRate = Rate;            // 초당 생성할 파티클 수
//    int32 SpawnCount = 0;              // 이번 프레임에 생성할 일반 파티클 수
//    float OldLeftover = SpawnFraction; // 지난 프레임에 못 만든 누적된 스폰 타이밍
//
//     // 2) 누적 & 정수/소수 분리
//    float Total = SpawnFraction + Rate * DeltaTime;
//    int32 ToSpawn = FMath::FloorToInt32(Total);
//    SpawnFraction = Total - ToSpawn;
//
//    // 3) 풀 크기 클램핑
//    ToSpawn = FMath::Min(ToSpawn, MaxActiveParticles - ActiveParticles);
//    if (ToSpawn <= 0)
//    {
//        return SpawnFraction;
//    }
//
//    // 4) 실제 생성
//    FVector Origin = Component->GetWorldLocation() + CurrentLODLevel->RequiredModule->EmitterOrigin;
//    const FVector InitialVelocity = FVector::ZeroVector;
//
//    /* Increment : 각 파티클 간의 시간 간격 (간격 시간) */
//    const float Increment = (Rate > 0.f) ? (1.f / Rate) : 0.f;
//
//    /* 현재 프레임 안에서 가장 늦게 생성될 파티클을 기준으로 StartTime 결정
//     * 그 후 각 파티클 마다 SpawnTime -= Increment 해가며 역순으로 분포
//     */
//
//    float StartTime = DeltaTime + SpawnFraction * Increment - Increment;
//
//    if ((SpawnRate > 0.f) /*|| (BurstCount > 0)*/) 
//    {
//        SpawnParticles(ToSpawn, StartTime,  Increment, Origin, 
//            InitialVelocity,nullptr
//        );
//    }
//
//    return SpawnFraction;
//}

float FParticleEmitterInstance::Spawn(float DeltaTime)
{
    // 1) continuous 스폰 계산
    UParticleModuleRequired* Req = CurrentLODLevel->RequiredModule;
    const float Duration = Req->EmitterDuration;

    float OldLeftover = SpawnFraction;
    float Total = OldLeftover + Req->SpawnRate * DeltaTime;
    int32 ToSpawn = FMath::FloorToInt32(Total);
    SpawnFraction = Total - ToSpawn;

    ToSpawn = FMath::Min(ToSpawn, MaxActiveParticles - ActiveParticles);

    // 2) burst 스폰 계산
    int32 BurstCount = 0;
    if (Duration > 0.f)
    {
        float OldT = FMath::Clamp(EmitterTime / Duration, 0.f, 1.f);
        float NewT = FMath::Clamp((EmitterTime + DeltaTime) / Duration, 0.f, 1.f);

        for (const FParticleBurst& Burst : Req->BurstList)
        {
            if (Burst.Time > OldT && Burst.Time <= NewT)
            {
                int32 Num = (Burst.CountLow >= 0)
                    ? FMath::FRandRange(Burst.CountLow, Burst.Count)
                    : Burst.Count;
                BurstCount += Num;
            }
        }
    }

    // advance & loop emitter time
    EmitterTime += DeltaTime;
    if (EmitterTime >= Duration && Duration > 0.f)
    {
        EmitterTime = FMath::Fmod(EmitterTime, Duration);
    }

    // 3) continuous 스폰
    if (ToSpawn > 0)
    {
        const FVector Origin = Component->GetWorldLocation() + Req->EmitterOrigin;
        const FVector InitialVelocity = FVector::ZeroVector;
        const float   Increment = (Req->SpawnRate > 0.f) ? (1.f / Req->SpawnRate) : 0.f;
        float         StartTime = DeltaTime + OldLeftover * Increment - Increment;

        SpawnParticles(ToSpawn, StartTime, Increment, Origin, InitialVelocity, nullptr);
    }

    // 4) burst 스폰
    if (BurstCount > 0)
    {
        const FVector Origin = Component->GetWorldLocation() + Req->EmitterOrigin;
        const FVector InitialVelocity = FVector::ZeroVector;
        // 보통 버스트는 한꺼번에, 시간 분산은 legacy 스폰 동작으로 처리할 수 있음
        SpawnParticles(BurstCount, 0.f, 0.f, Origin, InitialVelocity, nullptr);
    }

    return SpawnFraction;
}

/* 파티클 버퍼를 초기화하고 기본 위치, 속도를 세팅하는 함수  */
void FParticleEmitterInstance::PreSpawn(FBaseParticle* Particle, const FVector& InitialLocation, const FVector& InitialVelocity)
{
    FPlatformMemory::MemZero(Particle, ParticleSize);
    Particle->Location = InitialLocation;
    Particle->OldLocation = InitialLocation;
    Particle->Velocity = InitialVelocity;
    Particle->BaseVelocity = InitialVelocity;

    /*UE_LOG(ELogLevel::Error,
        TEXT("PreSpawn: Loc=(%.1f,%.1f,%.1f) Vel=(%.1f,%.1f,%.1f)"),
        InitialLocation.X, InitialLocation.Y, InitialLocation.Z,
        InitialVelocity.X, InitialVelocity.Y, InitialVelocity.Z);*/

    // 생명 시간
    Particle->RelativeTime = 0.0f;
    float& Lifetime = CurrentLODLevel->RequiredModule->EmitterDuration;
}

void FParticleEmitterInstance::PostSpawn(FBaseParticle* Particle, float InterpolationPercentage, float SpawnTime)
{
    // TODO : Spawn 시 위치를 보간 적용 필요 (위치가 1이상 차이날 경우)

    // 현재 스프라이트엔 특별 후처리 없음
    // Offset caused by any velocity
    Particle->OldLocation = Particle->Location;

	/* Spawn 시점이 프레임 시작과 달라 생기는 위치 보정, 현재는 적용 X  */
    //Particle->Location += SpawnTime * FVector(Particle->Velocity); // 다음 코드가

    // Store a sequence counter
    Particle->Flags |= STATE_Particle_JustSpawned;

}

void FParticleEmitterInstance::SpawnParticles(int32 Count, float StartTime, float Increment, const FVector& InitialLocation, const FVector& InitialVelocity, FParticleEventInstancePayload* EventPayload)
{
    // 오프셋 계산용 Base LOD
    UParticleLODLevel* HighLODLevel = SpriteTemplate->LODLevels[0];

    float SpawnTime = StartTime;
    float Interp = 1.0f;
    const float InterpDelta = (Count > 0 && Increment > 0.0f)
        ? (1.0f / (float)Count)
        : 0.0f;

     for (int32 Index = 0; Index < Count; Index++)
     {
         // 풀 한계 검사
         if (ActiveParticles >= MaxActiveParticles)
         {
             break;
         }


         /* 현재 파티클 인덱스 */
         /*const uint32 CurrIdx = ActiveParticles++;
         ParticleIndices[CurrIdx] = CurrIdx;*/

		 // 1) 이 프레임에 할당할 “데이터 인덱스” 꺼내기
		 const uint32 SlotIdx = ActiveParticles++;
		 const uint32 DataIdx = ParticleIndices[SlotIdx];

         // 실제 버퍼 위치 계산할때 DataIdx 사용
         DECLARE_PARTICLE_PTR(Particle, ParticleData + (ParticleStride * DataIdx));

         // 언리얼에선 bLegacySpawnBehavior가 true일때 아래와 같이 반영
         /* SpawnTime < 0 : 과거에 생성되었어야 하는 파티클
          * 균등 분포된 다수의 파티클 생성 시, 각 파티클을 프레임 시간 내에서 일정 간격으로 배치하기 위함
          * 한 프레임에 10개 생성 : 
          */
         SpawnTime -= Increment;
         Interp -= InterpDelta;

         PreSpawn(Particle, InitialLocation, InitialVelocity);
         //PreSpawn(Particle, Component->InitialLocationHardcoded, Component->InitialVelocityHardcoded);

         for (int32 ModuleIndex = 0; ModuleIndex < CurrentLODLevel->SpawnModules.Num(); ModuleIndex++)
         {
             UParticleModule* SpawnModule = CurrentLODLevel->SpawnModules[ModuleIndex];
             if (SpawnModule->bEnabled)
             {
                 UParticleModule* OffsetModule = HighLODLevel->SpawnModules[ModuleIndex]; //기존

                 // 모듈 자신을 넘겨야 올바른 오프셋이 리턴됨
                 uint32 DataOffset = GetModuleDataOffset(OffsetModule);
                 SpawnModule->Spawn(this, DataOffset, SpawnTime, Particle);
             }
         }

         

         // 3) VelocityModule 에서 누적된 최종 속도를
        //    Particle.Velocity 로부터 읽어서 Old/BaseVelocity 동기화
         Particle->OldLocation = Particle->Location;
         Particle->BaseVelocity = Particle->Velocity;

         
         PostSpawn(Particle, Interp, StartTime);

         if (Particle->RelativeTime > 1.0f)
         {
             //KillParticle(CurrIdx);

             // Process next particle
             continue;
         }

         /*UE_LOG(ELogLevel::Display,
             TEXT("Spawned #%d: Loc=(%.1f,%.1f,%.1f) Vel=(%.1f,%.1f,%.1f)"),
             Index + 1,
             Particle->Location.X, Particle->Location.Y, Particle->Location.Z,
             Particle->Velocity.X, Particle->Velocity.Y, Particle->Velocity.Z
         );

		 UE_LOG(ELogLevel::Display,
			 TEXT("Spawned #%d: SlotIdx=%d DataIdx=%d Active=%d"),
			 Index + 1, SlotIdx, DataIdx, ActiveParticles);*/
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

uint32 FParticleEmitterInstance::GetModuleDataOffset(UParticleModule* Module) const
{
    uint32* Offset = SpriteTemplate->ModuleOffsetMap.Find(Module);
    return (Offset != nullptr) ? *Offset : 0;
}

uint8* FParticleEmitterInstance::GetModuleInstanceData(UParticleModule* Module) const
{
    if (!Module || !InstanceData)
    {
        return nullptr;
    }

    const uint32* Offset = SpriteTemplate->ModuleInstanceOffsetMap.Find(Module);
    return Offset ? (InstanceData + *Offset) : nullptr;
}

/*
 * 반드시 CacheEmitterModuleInfo() 호출 후에 Init() 호출해야 SpriteTemplate의 유효한 값 참조 가능.. (ReqInstanceBytes, Map 등)
 */
void FParticleEmitterInstance::Init(UParticleSystemComponent* InComponent, int32 InEmitterIndex)
{
    Component = InComponent;
    SpriteTemplate = Component->Template->Emitters[InEmitterIndex];
    CurrentLODLevelIndex = 0;
    CurrentLODLevel = SpriteTemplate->LODLevels[0];
    bEnabled = true;

    // --- 1) 기본 파티클 크기 ---
    ParticleSize = sizeof(FBaseParticle);

    // --- 2) TypeData 추가 Payload 처리 ---
    if (UParticleModuleTypeDataBase* TypeData = CurrentLODLevel->TypeDataModule)
    {
        // 가상 함수 호출 (자식 클래스에서 override 되어야 함)
        const int32 TypeDataBytes = TypeData->RequiredBytes(nullptr);
        ParticleSize += TypeDataBytes;
    }

    // --- 3) 16바이트 정렬 (SIMD 최적화용) ---
    ParticleSize = Align(ParticleSize, 16); // 136 -> Align시 144 
    PayloadOffset = ParticleSize;

    // --- 4) 인스턴스 데이터 메모리 할당 ---
    const int32 InstanceSize = SpriteTemplate->ReqInstanceBytes;
    InstanceData = (uint8*)FPlatformMemory::Realloc<EAT_Container>(InstanceData, InstanceSize);
    if (InstanceData)
    {
        FPlatformMemory::MemZero(InstanceData, InstanceSize);
        // 현재 ModulesNeedingInstanceData Empty
        for (UParticleModule* Module : SpriteTemplate->ModulesNeedingInstanceData)
        {
            uint8* ModData = GetModuleInstanceData(Module);
            if (ModData)
            {
                Module->PrepPerInstanceBlock(this, ModData);
            }
        }
    }

    InstancePayloadSize = InstanceSize;

    // --- 5) 전체 ParticleStride 계산 ---
    ParticleStride = ParticleSize + InstancePayloadSize;

    // --- 6) 파티클 상태 초기화 ---
    ActiveParticles = 0;
    SpawnFraction = 0.f;
    EmitterTime = 0.f;
    ParticleCounter = 0;

    UpdateTransforms();
    Location = Component->GetWorldLocation();

    // --- 7) 최초 풀 할당 ---
    int32 InitialCount = SpriteTemplate->InitialAllocationCount > 0
        ? FMath::Min(SpriteTemplate->InitialAllocationCount, 100)
        : 10;

    Resize(InitialCount, /*bSetMaxActiveCount=*/true);
}

void FParticleEmitterInstance::UpdateTransforms()
{
    UParticleLODLevel* LODLevel = CurrentLODLevel;

    FMatrix ComponentToWorld = Component != nullptr ?
        Component->GetWorldMatrix().GetMatrixWithoutScale() : FMatrix::Identity;
    FMatrix EmitterToComponent = 
          FMatrix::CreateRotationMatrix(LODLevel->RequiredModule->EmitterRotation)
        * FMatrix::CreateTranslationMatrix(LODLevel->RequiredModule->EmitterOrigin);

    if (LODLevel->RequiredModule->bUseLocalSpace)
    {
        EmitterToSimulation = EmitterToComponent;
        SimulationToWorld = ComponentToWorld;
    }
    else
    {
        EmitterToSimulation = EmitterToComponent * ComponentToWorld;
        SimulationToWorld = FMatrix::Identity;
    }

}

/*
 * 재할당 후, 새로 들어오는 모든 파티클의 NewMaxActiveParticles 인덱스 초기화
 */
bool FParticleEmitterInstance::Resize(int32 NewMaxActiveParticles, bool bSetMaxActiveCount)
{
    if (NewMaxActiveParticles < MaxActiveParticles)
    {
        return true;
    }

    const int32 NewBytes = ParticleStride * NewMaxActiveParticles;
    ParticleData = (uint8*)FPlatformMemory::Realloc<EAT_Container>(ParticleData, NewBytes);


    FPlatformMemory::MemZero(
        ParticleData + (ParticleStride * MaxActiveParticles),
        (NewMaxActiveParticles - MaxActiveParticles) * ParticleStride
    );

	uint16* OldIndices = ParticleIndices;
    ParticleIndices = (uint16*)FPlatformMemory::Realloc<EAT_Container>(
        ParticleIndices,
        sizeof(uint16) * (NewMaxActiveParticles + 1)
    );

	// **1) 만약 처음 할당(OldIndices==nullptr)이면, 0..NewMax-1 전체를 채우고**
	// **2) 아니라면(늘어날 때)엔 old..new-1만 채워 주고**
	if (OldIndices == nullptr)
	{
		// 첫 할당인 경우
		for (int32 i = 0; i < NewMaxActiveParticles; ++i)
		{
			ParticleIndices[i] = i;
		}
	}
	else
	{
		// 단순 확대인 경우
		for (int32 i = MaxActiveParticles; i < NewMaxActiveParticles; ++i)
		{
			ParticleIndices[i] = i;
		}
	}

    MaxActiveParticles = NewMaxActiveParticles;

    if (bSetMaxActiveCount)
    {
        UParticleLODLevel* LOD0 = SpriteTemplate->LODLevels[0];
        LOD0->PeakActiveParticles = FMath::Max(MaxActiveParticles, LOD0->PeakActiveParticles);
    }

    return true;
}

void FParticleEmitterInstance::Rewind()
{
    // SecondsSinceCreation = 0;
    // EmitterTime = 0;
    // LoopCount = 0;
    ParticleCounter = 0;
    // bEnabled = 1;
}

void FParticleEmitterInstance::UpdateBoundingBox(float DeltaTime)
{
    // 단순 위치 업데이트만 수행
    auto* Owner = this;
    BEGIN_MY_UPDATE_LOOP
    {
        FVector Old = Particle.Location; // 디버깅용 변수

        // Freeze, JustSpawned 등 상태 무시하고 단순 업데이트만
        Particle.OldLocation = Particle.Location;
        Particle.Location += Particle.Velocity * DeltaTime;
        Particle.RelativeTime += DeltaTime * Particle.OneOverMaxLifetime;

        /*UE_LOG(ELogLevel::Error,
            TEXT("UpdateBoundingBox(): Index=%d, OldLoc=(%.2f, %.2f, %.2f), Vel=(%.2f, %.2f, %.2f), NewLoc=(%.2f, %.2f, %.2f), Life : (%.2f)"),
            i,
            Old.X, Old.Y, Old.Z,
            Particle.Velocity.X, Particle.Velocity.Y, Particle.Velocity.Z,
            Particle.Location.X, Particle.Location.Y, Particle.Location.Z, Particle.RelativeTime
        );*/
    }
    END_MY_UPDATE_LOOP;
}

/*
 * ParticleMeshEmitterInstance 초기화
 * 매번 캐스팅 연산을 줄이기 위해 템플릿을 미리 캐싱함
 */
void FParticleMeshEmitterInstance::Init(UParticleSystemComponent* InComponent, int32 InEmitterIndex)
{
    FParticleEmitterInstance::Init(InComponent, InEmitterIndex);

    UParticleLODLevel* LODLevel = SpriteTemplate->LODLevels[CurrentLODLevelIndex];
    MeshTypeData = Cast<UParticleModuleTypeDataMesh>(LODLevel->TypeDataModule);

    if (MeshTypeData->IsA(UParticleModuleTypeDataMesh::StaticClass()) == false)
    {
        UE_LOG(ELogLevel::Warning, TEXT("FParticleMeshEmitterInstance::Init() : TypeData is not Mesh"));
        return;
    }
    // if (MeshTypeData->Mesh) 추가 초기화 필요시 다음과 같이 작성 필요
}

FDynamicEmitterDataBase* FParticleMeshEmitterInstance::GetDynamicData(bool bSelected)
{
    UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
    if (!LODLevel || !bEnabled || !LODLevel->RequiredModule->bEnabled)
    {
        return nullptr;
    }

    FDynamicMeshEmitterData* NewMeshData = new FDynamicMeshEmitterData(LODLevel->RequiredModule);

    if (!FillReplayData(NewMeshData->Source))
    {
        delete NewMeshData;
        return nullptr;
    }

    /* [NOTE!!] : 제대로 복사되는지 확인 필요*/
    NewMeshData->StaticMesh = MeshTypeData->Mesh;

    // TODO : 원래 FillReplayData에 존재해야 함
    NewMeshData->Source.eEmitterType = DET_Mesh;

    // 선택 및 버퍼 생성
    NewMeshData->bValid = true;
    NewMeshData->bSelected = bSelected;

    // 3) DataContainer 내부 순회하며 파티클 위치/속도 로그 출력
    //{
    //    uint8* RawBuffer = NewMeshData->Source.DataContainer.ParticleData;
    //    int32        Stride = NewMeshData->Source.ParticleStride;
    //    int32        Count = NewMeshData->Source.ActiveParticleCount;
    //
    //    for (int32 i = 0; i < Count; ++i)
        //{
        //    FBaseParticle* P = reinterpret_cast<FBaseParticle*>(RawBuffer + NewMeshData->Source.DataContainer.ParticleIndices[i] * Stride);
        //    UE_LOG(ELogLevel::Error,
        //        TEXT("GetDynamicData(): Particle[%d] Loc=(%.2f, %.2f, %.2f) Vel=(%.2f, %.2f, %.2f)"),
        //        i,
        //        P->Location.X, P->Location.Y, P->Location.Z,
        //        P->Velocity.X, P->Velocity.Y, P->Velocity.Z);
        //}
    //}

    return NewMeshData;
}

bool FParticleMeshEmitterInstance::FillReplayData(FDynamicEmitterReplayDataBase& OutData)
{
    return Super::FillReplayData(OutData);
}

void FParticleSpriteEmitterInstance::Init(UParticleSystemComponent* InComponent, int32 InEmitterIndex)
{
    Super::Init(InComponent, InEmitterIndex);
}

FDynamicEmitterDataBase* FParticleSpriteEmitterInstance::GetDynamicData(bool bSelected)
{
    return Super::GetDynamicData(bSelected);
}

bool FParticleSpriteEmitterInstance::FillReplayData(FDynamicEmitterReplayDataBase& OutData)
{
    return Super::FillReplayData(OutData);
}


FDynamicEmitterDataBase::FDynamicEmitterDataBase(const UParticleModuleRequired* RequiredModule)
    : bSelected(false)
    , EmitterIndex(INDEX_NONE)
{

}

void FDynamicSpriteEmitterData::Init(bool bInSelected)
{
    bSelected = bInSelected;

    const int32 NumParticles = Source.ActiveParticleCount;
    const int32 NumVert      = NumParticles * 4;
    const int32 NumIdx       = NumParticles * 6;

    // 기존 버퍼 해제
    if (VertexBuffer)       { VertexBuffer->Release();       VertexBuffer = nullptr; }
    if (IndexBuffer)        { IndexBuffer->Release();        IndexBuffer = nullptr; }
    if (DynamicParamBuffer) { DynamicParamBuffer->Release(); DynamicParamBuffer = nullptr; }
    
    // 버퍼 생성
    D3D11_BUFFER_DESC desc = {};
    desc.Usage          = D3D11_USAGE_DYNAMIC;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    // VertexBuffer
    desc.BindFlags  = D3D11_BIND_VERTEX_BUFFER;
    desc.ByteWidth  = NumVert * GetDynamicVertexStride();
    FEngineLoop::GraphicDevice.Device->CreateBuffer(&desc, nullptr, &VertexBuffer);

    // IndexBuffer
    desc.BindFlags  = D3D11_BIND_INDEX_BUFFER;
    desc.ByteWidth  = NumIdx * sizeof(uint16);
    FEngineLoop::GraphicDevice.Device->CreateBuffer(&desc, nullptr, &IndexBuffer);

    // DynamicParamBuffer (필요 시)
    desc.BindFlags  = D3D11_BIND_CONSTANT_BUFFER;
    desc.ByteWidth  = NumParticles * GetDynamicParameterVertexStride();
    FEngineLoop::GraphicDevice.Device->CreateBuffer(&desc, nullptr, &DynamicParamBuffer);
}

bool FDynamicSpriteEmitterData::GetVertexAndIndexData(
    void* VertexData,
    void* DynamicParameterVertexData,
    void* FillIndexData,
    FParticleOrder* ParticleOrder,
    const FVector& InCameraPosition,
    const FMatrix& InLocalToWorld,
    uint32 InstanceFactor
) const
{
    auto* VertPtr = static_cast<FParticleSpriteVertex*>(VertexData);
    auto* ParamPtr = static_cast<FParticleVertexDynamicParameter*>(DynamicParameterVertexData);

    for (int32 i = 0; i < Source.ActiveParticleCount; ++i)
    {
        int32 ParticleIdx = ParticleOrder ? ParticleOrder[i].ParticleIndex : i;
        const uint8* ParticleBase = Source.DataContainer.ParticleData + ParticleIdx * Source.ParticleStride;

        // 1) FBaseParticle 정보 읽기 (Position, RelativeTime 등)
        FBaseParticle* Base = (FBaseParticle*)ParticleBase;
        FVector Pos         = InLocalToWorld.TransformPosition(Base->Location);
        float  RelTime     = Base->RelativeTime;

        // 2) 스프라이트 페이로드 읽기 (회전, 크기, SubUV 등)
        //    FSpriteParticlePayload* Payload = (FSpriteParticlePayload*)(ParticleBase + PayloadOffset);
        //    float Rot = Payload->Rotation; FVector2D Size = Payload->Size; uint8 SubImage = Payload->SubImageIndex;

        // 3) Instancing용 정점 채우기
        VertPtr[i].Position      = Pos;
        VertPtr[i].RelativeTime  = RelTime;
        VertPtr[i].OldPosition   = InLocalToWorld.TransformPosition(Base->OldLocation);
        VertPtr[i].ParticleId    = (float)ParticleIdx;
        VertPtr[i].Size          = /*Payload->Size*/ FVector2D(Base->Size.X, Base->Size.Y);
        VertPtr[i].Rotation      = /*Payload->Rotation*/ 0.0f;
        VertPtr[i].SubImageIndex = /*(float)Payload->SubImageIndex*/ 0.0f;
        VertPtr[i].Color         = Base->Color;

        // 4) DynamicParameter 채우기 (필요 시)
        // if (DynamicParameterVertexData)
        // {
        //     ParamPtr[i].DynamicValue[0] = 
        // }
    }
    return true;
}

bool FDynamicSpriteEmitterData::GetVertexAndIndexDataNonInstanced(
    void*           VertexData,
    void*           DynamicParameterVertexData,
    void*           FillIndexData,
    FParticleOrder* ParticleOrder,
    const FVector&  InCameraPosition,
    const FMatrix&  InLocalToWorld,
    int32           NumVerticesPerParticle
) const
{
    auto* VertPtr  = static_cast<FParticleSpriteVertexNonInstanced*>(VertexData);
    auto* IdxPtr   = static_cast<uint16*>(FillIndexData);

    const int32 Stride = Source.ParticleStride;
    for (int32 i = 0; i < Source.ActiveParticleCount; ++i)
    {
        int32 ParticleIdx = ParticleOrder ? ParticleOrder[i].ParticleIndex : i;
        const uint8* ParticleBase = Source.DataContainer.ParticleData + ParticleIdx * Stride;
        FBaseParticle* Base = (FBaseParticle*)ParticleBase;
        FVector Pos = InLocalToWorld.TransformPosition(Base->Location);
        // ... 스프라이트 페이로드 읽기 ...

        // 4개 정점 채우기
        for (int corner = 0; corner < 4; ++corner)
        {
            int32 VertIndex = i * 4 + corner;
            // VertPtr[VertIndex].UV = ...;
            // VertPtr[VertIndex].Position     = ...; // Pivot 오프셋, 회전 적용
            // VertPtr[VertIndex].RelativeTime = Base->RelativeTime;
            // ...
        }

        // 6개 인덱스 채우기 (두 삼각형)
        const int32 BaseVertexIndex = i * 4;
        IdxPtr[i*6 + 0] = BaseVertexIndex + 0;
        IdxPtr[i*6 + 1] = BaseVertexIndex + 1;
        IdxPtr[i*6 + 2] = BaseVertexIndex + 2;
        IdxPtr[i*6 + 3] = BaseVertexIndex + 2;
        IdxPtr[i*6 + 4] = BaseVertexIndex + 1;
        IdxPtr[i*6 + 5] = BaseVertexIndex + 3;
    }
    return true;
}

FDynamicMeshEmitterData::FDynamicMeshEmitterData(const UParticleModuleRequired* RequiredModule)
    : FDynamicSpriteEmitterData(RequiredModule)
{
}

void FDynamicMeshEmitterData::Init(bool bInSelected)
{
    FDynamicSpriteEmitterData::Init(bInSelected);
}

bool FDynamicMeshEmitterData::GetVertexAndIndexData(
    void* VertexData,
    void* /*DynamicParameterVertexData*/,
    void* /*FillIndexData*/,
    FParticleOrder* ParticleOrder,
    const FVector& /*InCameraPosition*/,
    const FMatrix& InLocalToWorld,
    uint32 /*InstanceFactor*/
) const
{
    // Per-instance로 보낼 데이터만 채워줍니다.
    // VertexData는 FMeshParticleInstanceVertex 배열이어야 합니다.
    auto* Out = static_cast<FMeshParticleInstanceVertex*>(VertexData);
    const int32 Num = Source.ActiveParticleCount;
    const auto& DC = Source.DataContainer;

    for (int32 i = 0; i < Num; ++i)
    {
        int32 PartIdx = ParticleOrder ? ParticleOrder[i].ParticleIndex : i;
        int32 DataIdx = DC.ParticleIndices[PartIdx];
        auto* Base = reinterpret_cast<FBaseParticle*>(
            DC.ParticleData + DataIdx * Source.ParticleStride);

        // 월드 매트릭스: 시뮬레이션→월드와 파티클 위치
        FMatrix M = FMatrix::CreateTranslationMatrix(Base->Location).GetMatrixWithoutScale() /** InLocalToWorld*/;

        // 채우기
        Out[i].Transform = M;                           // 4×4
        Out[i].Color = Base->Color;                // 파티클 컬러
        Out[i].Velocity = FVector4(Base->Velocity, 0); // 속도
        //Out[i].SubUVParams[0] = 0; Out[i].SubUVParams[1] = 0;
        //Out[i].SubUVParams[2] = 0; Out[i].SubUVParams[3] = 0;
        //Out[i].SubUVLerp = 0.f;
        //Out[i].RelativeTime = Base->RelativeTime;
    }

    return true;
}

FDynamicSpriteEmitterReplayDataBase::FDynamicSpriteEmitterReplayDataBase()
    //: MaterialInterface(nullptr)
    : RequiredModule(nullptr)
    , NormalsSphereCenter(FVector::ZeroVector)
    , NormalsCylinderDirection(FVector::ZeroVector)
    , InvDeltaSeconds(0.0f)
    , MaxDrawCount(0)
    , OrbitModuleOffset(0)
    , DynamicParameterDataOffset(0)
    , LightDataOffset(0)
    , LightVolumetricScatteringIntensity(0)
    , CameraPayloadOffset(0)
    , SubUVDataOffset(0)
    , SubImages_Horizontal(1)
    , SubImages_Vertical(1)
    , bUseLocalSpace(false)
    , bLockAxis(false)
    , ScreenAlignment(0)
    , LockAxisFlag(0)
    , EmitterRenderMode(0)
    , EmitterNormalsMode(0)
    , PivotOffset(-0.5f, -0.5f)
    , bUseVelocityForMotionBlur(false)
    , bRemoveHMDRoll(false)
    , MinFacingCameraBlendDistance(0.f)
    , MaxFacingCameraBlendDistance(0.f)
{
}


FDynamicSpriteEmitterReplayDataBase::~FDynamicSpriteEmitterReplayDataBase()
{
    //delete RequiredModule;
}

/** FDynamicSpriteEmitterReplayDataBase Serialization */
void FDynamicSpriteEmitterReplayDataBase::Serialize(FArchive& Ar)
{
    // Call parent implementation
    FDynamicEmitterReplayDataBase::Serialize(Ar);

    Ar << ScreenAlignment;
    Ar << bUseLocalSpace;
    Ar << bLockAxis;
    Ar << LockAxisFlag;
    Ar << MaxDrawCount;

    int32 EmitterRenderModeInt = EmitterRenderMode;
    Ar << EmitterRenderModeInt;
    EmitterRenderMode = EmitterRenderModeInt;

    Ar << OrbitModuleOffset;
    Ar << DynamicParameterDataOffset;
    Ar << LightDataOffset;
    Ar << LightVolumetricScatteringIntensity;
    Ar << CameraPayloadOffset;

    Ar << EmitterNormalsMode;
    Ar << NormalsSphereCenter;
    Ar << NormalsCylinderDirection;

    //Ar << MaterialInterface;

    Ar << PivotOffset;

    Ar << bUseVelocityForMotionBlur;
    Ar << bRemoveHMDRoll;
    Ar << MinFacingCameraBlendDistance;
    Ar << MaxFacingCameraBlendDistance;
}
