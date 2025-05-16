#include "ParticleSystemComponent.h"

#include "ParticleEmitterInstances.h"
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

    ComputeTickComponent_Concurrent(DeltaTime);

    
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
            //UParticleLODLevel* SpriteLODLevel = Instance->SpriteTemplate->GetCurrentLODLevel(Instance);
            //// [간략] : Significance 관리 없이 Tick 수행
            //if (SpriteLODLevel && SpriteLODLevel->bEnabled)
            //{
            //    Instance->Tick(DeltaTimeTick, bSuppressSpawning);
            //    // Tick_MaterialOverrides(EmitterIndex);
            //}
        }
    }
}
