#include "ParticleSystemComponent.h"

#include "ParticleEmitter.h"
#include "ParticleEmitterInstances.h"
#include "ParticleLODLevel.h"
#include "ParticleModuleRequired.h"
#include "ParticleSystem.h"
#include "World/World.h"

void UParticleSystemComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    if (Template == nullptr /*|| Template->Emitters.Num() == 0*/)
    {
        return; // SetComponentTickEnabled(false);
        
    }

    if (GetWorld()->IsGameWorld())
    {
        //for (APlayerController*& : TObjectRange<>)
    }

    // 최초 한 번만 파티클 이터레이터 인스턴스 생성
    if (EmitterInstances.Num() == 0)
    {
        InitParticles();
    }


    ComputeTickComponent_Concurrent(DeltaTime);

    if (IsActive())
    {
        EmitterRenderData.Empty();
        // [언리얼] : CPU의 파티클 데이터를 렌더 스레드에 전달하는 Send.._Concurrent() 존재
        for (FParticleEmitterInstance*& Inst : EmitterInstances)
        {
            FDynamicEmitterDataBase* Dyn = Inst->GetDynamicData(false);
            if (Dyn)
            {
                EmitterRenderData.Add(Dyn);
            }
        }
    }

}

void UParticleSystemComponent::ResetParticles()
{
    for (FParticleEmitterInstance* OldInstance : EmitterInstances)
    {
        if (OldInstance)
        {
            delete OldInstance;
            OldInstance = nullptr;
        }
    }
    EmitterInstances.Empty();
}

void UParticleSystemComponent::InitParticles()
{
    ResetParticles();

    const int32 NumEmitters = Template->Emitters.Num();
    EmitterInstances.Reserve(NumEmitters);

    for (int32 EmitterIndex = 0; EmitterIndex < NumEmitters; EmitterIndex++)
    {
        UParticleEmitter* EmitterTemplate = Template->Emitters[EmitterIndex];
        if (!EmitterTemplate)
        {
            continue;
        }

        UParticleModuleRequired* Required = EmitterTemplate->LODLevels[0]->RequiredModule;
        int32 MaxParts = 0;

        
        FParticleEmitterInstance* Inst = new FParticleEmitterInstance();

        Inst->Component = this;
        Inst->SpriteTemplate = EmitterTemplate;

        Inst->CurrentLODLevelIndex = 0;
        Inst->CurrentLODLevel = EmitterTemplate->LODLevels[0]; // LOD 기본 0


        //Inst->MaxActiveParticles = MaxParts;
        Inst->ParticleSize = sizeof(FBaseParticle);
        //Inst->InstancePayloadSize = Required->InstancePayloadSize;
        Inst->ParticleStride = Inst->ParticleSize + Inst->InstancePayloadSize;
        Inst->PayloadOffset = Inst->ParticleSize;

        // 파티클 풀 메모리 할당
        Inst->ParticleData = (uint8*)malloc(Inst->ParticleStride * MaxParts);
        Inst->ParticleIndices = (uint16*)malloc(sizeof(uint16) * MaxParts);
        Inst->InstanceData = nullptr;

        // 초기 파티클 상태
        Inst->ActiveParticles = 0;
        Inst->SpawnFraction = 0.f;
        Inst->EmitterTime = 0.f;
        Inst->bEnabled = true;

        EmitterInstances.Add(Inst);
        
    }
}

/* 파티클 시스템을 더 이상 활성 상태로 두지 않음을 의미
 * 1. 더이상 파티클을 생성하지 않음
 * 2. bKillOnDeactivate에 따라 EmitterInstance를 즉시 삭제 / 잔여 파티클 자연 소멸
 */
void UParticleSystemComponent::DeactivateSystem()
{
    //
    for (int32 i = 0; i < EmitterInstances.Num(); i++)
    {
        FParticleEmitterInstance* Instance = EmitterInstances[i];
        // if instance bKillOnDeactivate is true, kill it
        if (Instance)
        {
            // clean up other instances that may point to this one
            // ... //

            delete Instance;
            EmitterInstances[i] = nullptr;
        }
        else
        {
            //Instance->OnDeactivateSystem();
        }
    }
}

/*
 * @brief TickComponent_Concurrent
 *
 * 주요 : 모든 Emitter를 순회하며 1) LOD 선택, 2) 파티클 Tick 을 수행
 * = 한 프레임 동안의 파티클 시뮬레이션을 수행
 * @param DeltaTimeTick : Tick에 소요된 시간 (원랜 DeltaTimeTick 변수로 존재)
 */
void UParticleSystemComponent::ComputeTickComponent_Concurrent(float DeltaTimeTick)
{
    for (int32 EmitterIndex = 0; EmitterIndex < EmitterInstances.Num(); EmitterIndex++)
    {
        FParticleEmitterInstance* Instance = EmitterInstances[EmitterIndex];
        if (Instance && Instance->SpriteTemplate)
        {

            // 1) Emitter가 어떤 LOD를 사용할지 결정
            UParticleLODLevel* SpriteLODLevel = Instance->SpriteTemplate->GetCurrentLODLevel(Instance);
            // [간략] : Significance 관리 없이 Tick 수행
            if (SpriteLODLevel && SpriteLODLevel->bEnabled)
            {
                Instance->Tick(DeltaTimeTick, bSuppressSpawning);
                // Tick_MaterialOverrides(EmitterIndex);
            }
        }
    }
}


/**
* 정적 메서드: 리플레이 데이터를 제공받아, 파티클 시스템을 렌더링하는 데 사용할 수 있는 동적 Emitter 데이터를 생성
*
* @param	EmitterInstance		이 리플레이가 실행 중인 Emitter 인스턴스
* @param	EmitterReplayData	들어오는 리플레이 데이터 (NULL이 될 수 없음)
* @param	bSelected			파티클 시스템이 현재 선택된 상태라면 true
*
* @return	새로 생성된 동적 데이터, 실패 시 NULL 반환
*/
FDynamicEmitterDataBase* UParticleSystemComponent::CreateDynamicDataFromReplay(
    FParticleEmitterInstance* EmitterInstance, const FDynamicEmitterReplayDataBase* EmitterReplayData, bool bSelected
)
{

    FDynamicEmitterDataBase* DynData = nullptr;
    switch (EmitterReplayData->eEmitterType)
    {
    case DET_Sprite:
    {
        auto* SpriteData = new FDynamicSpriteEmitterData(
            EmitterInstance->CurrentLODLevel->RequiredModule
        );

        // Replay → Source 복사
        SpriteData->Source = *static_cast<const FDynamicSpriteEmitterReplayDataBase*>(EmitterReplayData);
        SpriteData->bSelected = bSelected;
        SpriteData->Init(bSelected);  // 정렬, 버텍스/인덱스 생성
        DynData = SpriteData;
        break;
    }
    default:
        break;

        // …Mesh, Ribbon 등 다른 타입도 마찬가지…
    }

    return DynData;
}
