#include "ParticleModule.h"
#include "TypeData/ParticleModuleTypeDataBase.h"

#include "UObject/Casts.h"

#include "ParticleModuleLifeTimeBase.h"
#include "ParticleModuleLifeTime.h"
#include "ParticleModuleColorBase.h"
#include "ParticleModuleColor.h"


void UParticleModule::PostInitProperties()
{
    bEnabled = true;
}

UObject* UParticleModule::Duplicate(UObject* InOuter)
{
    UParticleModule* NewModule = Cast<UParticleModule>(Super::Duplicate(InOuter));
    if (NewModule)
    {
        NewModule->bEnabled = bEnabled;
        NewModule->bSpawnModule = bSpawnModule;
        NewModule->bUpdateModule = bUpdateModule;
        NewModule->bFinalUpdateModule = bFinalUpdateModule;
        NewModule->bUpdateForGPUEmitter = bUpdateForGPUEmitter;
        NewModule->bCurvesAsColor = bCurvesAsColor;
        NewModule->b3DDrawMode = b3DDrawMode;
        NewModule->bSupported3DDrawMode = bSupported3DDrawMode;
        NewModule->EditorColor = EditorColor;
        NewModule->bEditable = bEditable;
    }
    return NewModule;
}

void UParticleModule::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
}

void UParticleModule::Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime)
{
}


void UParticleModule::FinalUpdate(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime)
{
}

/* UParticleModuleCollision로 주석 예시 작성
 * 아래 두 함수는, [FBaseParticle에 정의된 필드만을 사용하는 기본 모듈 기준] 모두 0을 반환해야 함 
 */

uint32 UParticleModule::RequiredBytes(UParticleModuleTypeDataBase* TypeData)
{
    return 0;
    // return sizeof(FParticleCollisionPayload);
}

/*
* 각 UParticleModule이 자신의 per - instance 데이터 크기를 리턴하는 함수
* 보통 SpawnModule, SizeModule, ColorModule 등은 0을 리턴(데이터를 가지지 않음)
* 일부 모듈(예: RibbonTrail, Mesh 등)은 데이터가 필요하여 > 0 값 반환
*/
uint32 UParticleModule::RequiredBytesPerInstance()
{
    return 0;
    // return sizeof(FParticleCollisionInstancePayload);
}
uint32 UParticleModule::PrepPerInstanceBlock(FParticleEmitterInstance* Owner, void* InstData)
{
    return 0xffffffff;
    // FParticleCollisionInstancePayload* CollisionInstPayload = (FParticleCollisionInstancePayload*)(InstData);
    // CollisionInstPayload->CurrentLODBoundsCheckCount = 0;
    // return 0;
}

float UParticleModuleLifeTimeBase::GetLifetimeValue(FParticleEmitterInstance* Owner, float InTime, UObject* Data)
{
    return 0.0f;
}
