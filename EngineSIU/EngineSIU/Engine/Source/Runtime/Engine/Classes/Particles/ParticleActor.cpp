#include "ParticleActor.h"

#include "ParticleModuleRequired.h"
#include "ParticleSystemComponent.h"
#include "ParticleSystem.h"
#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"
#include "ParticleModuleLocation.h"
#include "ParticleModuleTypeDataBase.h"
#include "ParticleModuleVelocity.h"

void AParticleActor::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();

    ParticleSystemComponent = AddComponent<UParticleSystemComponent>(FName("ParticleSystemComponent_0"));

    // -- 1) PS / Emitter / LOD 생성
    UParticleSystem* PS = FObjectFactory::ConstructObject<UParticleSystem>(this);
    UParticleEmitter* Emitter = FObjectFactory::ConstructObject<UParticleEmitter>(nullptr);
    UParticleLODLevel* LOD = FObjectFactory::ConstructObject<UParticleLODLevel>(nullptr);

    // -- 2) Required module
    UParticleModuleRequired* ReqMod = FObjectFactory::ConstructObject<UParticleModuleRequired>(nullptr);
    ReqMod->bEnabled = true;
    ReqMod->EmitterDuration = 3.0f;
    ReqMod->SpawnRate = 10.0f / ReqMod->EmitterDuration;  // 3초의 Emitter Cycle동안 1개의 파티클 생성

    // TypeDataModule 생성 및 세팅
    UParticleModuleTypeDataBase* TypeData = FObjectFactory::ConstructObject<UParticleModuleTypeDataBase>(nullptr);
    TypeData->bEnabled = true;

    // Location 모듈 생성
    UParticleModuleLocation* LocMod = FObjectFactory::ConstructObject<UParticleModuleLocation>(nullptr);
    LocMod->bEnabled = true;
    LocMod->StartLocation = FVector::ZeroVector;  // 하드코딩

    // Velocity 모듈 생성
    UParticleModuleVelocity* VelMod = FObjectFactory::ConstructObject<UParticleModuleVelocity>(nullptr);
    VelMod->bEnabled = true;
    //VelMod->StartVelocity = FVector(0.f, 0.f, 1.f);  // 하드코딩
    

    // TODO : (원한다면 파생 클래스로 교체: UParticleModuleTypeDataSprite 등)

    // 생성한 객체들 서로 연결
    PS->Emitters.Add(Emitter);
    Emitter->LODLevels.Add(LOD);

    LOD->RequiredModule = ReqMod;
    LOD->TypeDataModule = TypeData;

    LOD->Modules = { ReqMod, TypeData, LocMod, VelMod };
    LOD->SpawnModules = { ReqMod, TypeData, LocMod, VelMod };  // Spawn 시에만 위치+속도 세팅
    LOD->UpdateModules = { TypeData };

    // Offset/Size 계산
    PS->BuildEmitters();

    // 컴포넌트에 템플릿 연결 + 초기화
    ParticleSystemComponent->Template = PS;
    ParticleSystemComponent->InitializeSystem();

    // 하드코딩된 초기 위치/속도 세팅
    ParticleSystemComponent->InitialLocationHardcoded = FVector::ZeroVector;
    ParticleSystemComponent->InitialVelocityHardcoded = FVector(0.f, 0.f, 1.f);

    UE_LOG(ELogLevel::Error,
        TEXT("Initialized: Duration=%.1f, Rate=%.1f, Mods=%d"),
        ReqMod->EmitterDuration,
        ReqMod->SpawnRate,
        LOD->Modules.Num()
    );

}

UObject* AParticleActor::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewActor->ParticleSystemComponent = Cast<UParticleSystemComponent>(ParticleSystemComponent->Duplicate(InOuter));


    return NewActor;
}

void AParticleActor::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(ELogLevel::Error, TEXT("AParticleActor::BeginPlay - Simulation started"));
}
