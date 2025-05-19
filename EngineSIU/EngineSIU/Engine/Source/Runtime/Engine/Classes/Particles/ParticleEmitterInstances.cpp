#include "ParticleEmitterInstances.h"

#include <algorithm>

#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"
#include "ParticleModuleRequired.h"
#include "ParticleModuleTypeDataBase.h"
#include "ParticleSystem.h"
#include "ParticleSystemComponent.h"
#include "Templates/AlignmentTemplates.h"

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
 */
void FParticleEmitterInstance::KillParticles()
{
    for (int32 i = ActiveParticles - 1; i >= 0; --i)
    {
        FBaseParticle* Particle = (FBaseParticle*)(ParticleData + i * ParticleStride);
        if (Particle->RelativeTime >= 1.0f)
        {
            // KillCurrentParticle(ParticleIndices[i]);
            ParticleIndices[i] = ParticleIndices[ActiveParticles - 1];
            ParticleIndices[ActiveParticles - 1] = i;
            --ActiveParticles;
        }
    }
}

/* 인덱스 풀 방식 구현 : 인덱스만 스왑, 메모리 블록을 Memcpy로 앞당기지 않음 */
void FParticleEmitterInstance::KillParticle(int32 Index)
{
    if (Index >= 0 && Index < ActiveParticles)
    {
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
    , MaxActiveParticles(10)
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

    NewEmitterData->bSelected = bSelected;
    NewEmitterData->Init(bSelected);

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

    // 2) 공통 필드 복사
    OutData.eEmitterType = DET_Sprite;             // 스프라이트로 기본 가정
    OutData.ActiveParticleCount = ActiveParticles; // 활성 파티클 수
    OutData.ParticleStride = ParticleStride;       // 파티클 하나당 데이터 크기
    OutData.Scale = FVector(1, 1, 1);       // 기본 (1,1,1)
    OutData.SortMode = PSORTMODE_DistanceToView;   // 거리 기준 정렬 예시

    // ParticleStride : 파티클 하나당 바이트 수
    int32 TotalDataBytes = ParticleStride * ActiveParticles;    // 총 바이트 수 
    int32 TotalIndices = ActiveParticles;                       // 인덱스 개수
    OutData.DataContainer.Alloc(TotalDataBytes, TotalIndices);

    // 실제 데이터 복사 : OutData.DataContainer로 파티클 데이터 / 파티클 인덱스 깊은 복사
    memcpy(OutData.DataContainer.ParticleData, ParticleData, TotalDataBytes);
    memcpy(OutData.DataContainer.ParticleIndices, ParticleIndices, TotalIndices * sizeof(uint16));

    return true;
}

void FParticleEmitterInstance::Tick(float DeltaTime, bool bSuppressSpawning)
{
    UE_LOG(ELogLevel::Error, "Tick() Start: DeltaTime=%.4f, Active=%d", DeltaTime, ActiveParticles);

    UParticleLODLevel* LODLevel = SpriteTemplate->GetCurrentLODLevel(this);
    float EmitterDelay = Tick_EmitterTimeSetup(DeltaTime, LODLevel);

    if (bEnabled)
    {
        bool bFirstTime = (EmitterTime == 0.0f);

        // 수명이 다했거나 RelativeTime > 1.0 인 파티클을 제거.
        // 이유 : Kill before the spawn... Otherwise, we can get 'flashing' (beam2Emitter)
        KillParticles();

        UE_LOG(ELogLevel::Error, "  Before Update: Loc=%s", *Location.ToString());

        Tick_ModuleUpdate(DeltaTime, LODLevel);

        UE_LOG(ELogLevel::Error, "  After Update: Loc=%s", *Location.ToString());
        SpawnFraction = Tick_SpawnParticles(DeltaTime, LODLevel, bSuppressSpawning, bFirstTime);

        UE_LOG(ELogLevel::Error, "  SpawnFraction=%.4f", SpawnFraction);

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

        UE_LOG(ELogLevel::Error, "Tick() End : EmitterTime=%.4f, ActiveParticles=%d\n", EmitterTime, ActiveParticles);
    }

}

// 파티클 시스템 전체의 진행 시간 증가 및 딜레이, 루프, 종료 조건 처리
// @return : 실제 시뮬레이션에 쓰이지 않은 시간(딜레이)
float FParticleEmitterInstance::Tick_EmitterTimeSetup(float DeltaTime, UParticleLODLevel* InCurrentLODLevel)
{
    UpdateTransforms();
    // 현재는 단순화함 : Delay, Looping 무시
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
float FParticleEmitterInstance::Spawn(float DeltaTime)
{
    // SpawnRate 기반 파티클 생성 수 계산
    // 생성할 파티클 수 결정 시, SpawnParticles()로 실제 생성 (일반용, Burst용)
    // 누적되지 못한 잔여 시간(SpawnFraction) 다음 프레임으로 넘김

    // --- 1) SpawnRate 가져오기
    UParticleModuleRequired* Req = CurrentLODLevel->RequiredModule;
    float Rate = CurrentLODLevel->RequiredModule->SpawnRate; // Distribution 대신 고정 값

    float SpawnRate = Rate;            // 초당 생성할 파티클 수
    int32 SpawnCount = 0;              // 이번 프레임에 생성할 일반 파티클 수
    float OldLeftover = SpawnFraction; // 지난 프레임에 못 만든 누적된 스폰 타이밍

    // 간략화 버전
    // 잔여 분수 관리 (OldLeftOver, NewLeftOver)
    // 정수 파티클 개수 분리 (StartTime, Increment 계산 등)
    // 풀 크기 클램핑 (Resize)
    // 최대치 검사 (미리 할당된 파티클 풀 크기 내에서만 생성되도록 생성개수 클램핑)

     // 2) 누적 & 정수/소수 분리
    float Total = SpawnFraction + Rate * DeltaTime;
    int32 ToSpawn = FMath::FloorToInt32(Total);
    SpawnFraction = Total - ToSpawn;

    // 3) 풀 크기 클램핑
    ToSpawn = FMath::Min(ToSpawn, MaxActiveParticles - ActiveParticles);
    if (ToSpawn <= 0)
    {
        return SpawnFraction;
    }

    // 4) 실제 생성
    FVector Origin = Component->GetWorldLocation() + CurrentLODLevel->RequiredModule->EmitterOrigin;
    const FVector InitialVelocity = FVector::ZeroVector;

    // SpawnTime 분배: 균일 분포
    const float Increment = (Rate > 0.f) ? (1.f / Rate) : 0.f;
    float StartTime = DeltaTime + SpawnFraction * Increment - Increment;

    if ((SpawnRate > 0.f) /*|| (BurstCount > 0)*/) 
    {
        SpawnParticles(
            ToSpawn,
            StartTime,
            Increment,
            Origin,
            InitialVelocity,
            /*EventPayload=*/ nullptr
        );
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

/* 파티클 버퍼를 초기화하고 기본 위치, 속도를 세팅하는 함수  */
void FParticleEmitterInstance::PreSpawn(FBaseParticle* Particle, const FVector& InitialLocation, const FVector& InitialVelocity)
{

    FPlatformMemory::MemZero(Particle, ParticleSize);
    Particle->Location = InitialLocation;
    Particle->OldLocation = InitialLocation;
    Particle->Velocity = InitialVelocity;
    Particle->BaseVelocity = InitialVelocity;

    UE_LOG(ELogLevel::Error,
        TEXT("PreSpawn: Loc=(%.1f,%.1f,%.1f) Vel=(%.1f,%.1f,%.1f)"),
        InitialLocation.X, InitialLocation.Y, InitialLocation.Z,
        InitialVelocity.X, InitialVelocity.Y, InitialVelocity.Z);

    // 생명 시간
    Particle->RelativeTime = 0.0f;
    Particle->OneOverMaxLifetime = CurrentLODLevel->RequiredModule->EmitterDuration > 0.f
        ? 1.f / CurrentLODLevel->RequiredModule->EmitterDuration
        : 1.f;
}

void FParticleEmitterInstance::PostSpawn(FBaseParticle* Particle, float InterpolationPercentage, float SpawnTime)
{
    // TODO : Spawn 시 위치를 보간 적용 필요 (위치가 1이상 차이날 경우)

    // 현재 스프라이트엔 특별 후처리 없음
    // Offset caused by any velocity
    Particle->OldLocation = Particle->Location;
    Particle->Location += SpawnTime * FVector(Particle->Velocity);

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

         // Macro 추가 필요.
         DECLARE_PARTICLE_PTR(Particle, ParticleData + (ParticleStride * Index));
         const uint32 CurrentParticleIndex = ActiveParticles++;

         // 언리얼에선 bLegacySpawnBehavior가 true일때 아래와 같이 반영
         SpawnTime -= Increment;
         Interp -= InterpDelta;

         //PreSpawn(Particle, InitialLocation, InitialVelocity);
         PreSpawn(Particle, Component->InitialLocationHardcoded, Component->InitialVelocityHardcoded);

         for (int32 ModuleIndex = 0; ModuleIndex < CurrentLODLevel->SpawnModules.Num(); ModuleIndex++)
         {
             UParticleModule* SpawnModule = CurrentLODLevel->SpawnModules[ModuleIndex];
             if (SpawnModule->bEnabled)
             {
                 UParticleModule* OffsetModule = HighLODLevel->SpawnModules[ModuleIndex];
                 SpawnModule->Spawn(this, GetModuleDataOffset(OffsetModule), SpawnTime, Particle);
             }
         }
         
         PostSpawn(Particle, Interp, StartTime);

         if (Particle->RelativeTime > 1.0f)
         {
             KillParticle(CurrentParticleIndex);

             // Process next particle
             continue;
         }
         

         UE_LOG(ELogLevel::Display, "SpawnParticles() : Particle %d Spawned", Index+1);
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

    ParticleIndices = (uint16*)FPlatformMemory::Realloc<EAT_Container>(
        ParticleIndices,
        sizeof(uint16) * (NewMaxActiveParticles + 1)
    );

    for (int32 i = MaxActiveParticles; i < NewMaxActiveParticles; ++i)
    {
        ParticleIndices[i] = i;
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

        UE_LOG(ELogLevel::Error,
            TEXT("UpdateBoundingBox(): Index=%d, OldLoc=(%.2f, %.2f, %.2f), Vel=(%.2f, %.2f, %.2f), NewLoc=(%.2f, %.2f, %.2f), Life : (%.2f)"),
            i,
            Old.X, Old.Y, Old.Z,
            Particle.Velocity.X, Particle.Velocity.Y, Particle.Velocity.Z,
            Particle.Location.X, Particle.Location.Y, Particle.Location.Z, Particle.RelativeTime
        );
    }
    END_MY_UPDATE_LOOP;
}

FDynamicEmitterDataBase::FDynamicEmitterDataBase(const UParticleModuleRequired* RequiredModule)
    : bSelected(false)
    , EmitterIndex(INDEX_NONE)
{
}

void FDynamicSpriteEmitterData::Init(bool bInSelected)
{
    bSelected = bInSelected;
    // Material NULL Slot
    // TODO : 정렬, VertexBuffer 생성, BuildIndexBuffer 필요
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
