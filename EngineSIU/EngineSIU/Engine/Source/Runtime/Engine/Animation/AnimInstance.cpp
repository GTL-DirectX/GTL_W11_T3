#include "AnimInstance.h"

#include "AnimationStateMachine.h"
#include "AnimSequenceBase.h"
#include "AnimData/AnimDataModel.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/Mesh/SkeletalMesh.h"
#include "Engine/Asset/SkeletalMeshAsset.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Math/Transform.h"
#include "UObject/Casts.h"
#include "AnimTypes.h"
#include "AnimNotifies/AnimNotifyState.h"

UAnimInstance::UAnimInstance()
    : Sequence(nullptr)
    , OwningComp(nullptr)
    , CurrentTime(0.0f)
    , bPlaying(false)
{
    
}

void UAnimInstance::InitializeAnimation(USkeletalMeshComponent* InOwningComponent)
{
    OwningComp = InOwningComponent;
    if (AnimSM == nullptr)
    {
        AnimSM = FObjectFactory::ConstructObject<UAnimationStateMachine>(this);
    }
    NativeInitializeAnimation();
}


void UAnimInstance::UpdateAnimation(float DeltaSeconds)
{
    // 업데이트 전 네이티브 먼저 실행
    NativeUpdateAnimation(DeltaSeconds);
    
    if (!bPlaying || !Sequence || !OwningComp || !OwningComp->GetSkeletalMesh())
    {
        // 재생 중이 아니거나, 필요한 정보가 없으면 현재 OutPose를 변경하지 않거나 참조 포즈로 설정
        ResetToRefPose();
        return;
    }

    // 사용자 확장 영역 - 커스텀 변수 또는 입력 값 설정
    TriggerAnimNotifies(DeltaSeconds);
    NotifyQueue.Reset();

    //// 1. 애니메이션 시간 업데이트
    //const float RateScale = Sequence->GetRateScale();
    //CurrentTime += DeltaSeconds * RateScale;

    if (AnimSM->PendingTransition) {
        StartTransition();
    }

    if (bInBlend) {
        UpdateBlendTime(DeltaSeconds);
    }

    else {
        if (Sequence) {
            CurrentTime += DeltaSeconds * Sequence->GetRateScale();

            // FIXING : 제거해도 되는 부분 확인하기
            if (UAnimSequenceBase* GetSequence = AnimSM->GetCurrentAnimationSequence())
            {
                if (Sequence != GetSequence)
                {
                    CurrentTime = 0.0f;
                }

                Sequence = AnimSM->GetCurrentAnimationSequence();
                bPlaying = true;
            }

            const float SequenceLength = Sequence->GetPlayLength();
            const bool bLooping = Sequence->IsLooping();

            if (bLooping)
            {
                CurrentTime = FMath::Fmod(CurrentTime, SequenceLength);
                if (CurrentTime < 0.0f)
                {
                    CurrentTime = SequenceLength - FMath::Fmod(-CurrentTime, SequenceLength);
                }
            }
            else // 루핑이 아닐 때
            {
                if (CurrentTime >= SequenceLength)
                {
                    CurrentTime = SequenceLength; // 마지막 프레임에서 멈춤, 재생 종료
                    SetPlaying(false);
                }
                else if (CurrentTime < 0.0f) // 음수 시간 처리 (RateScale이 음수일 경우)
                {
                    CurrentTime = 0.0f;
                    SetPlaying(false);
                }
            }
        }
    }
}

const TArray<FTransform>& UAnimInstance::EvaluateAnimation()
{
    USkeletalMesh* SkelMesh = OwningComp->GetSkeletalMesh();
    if (!SkelMesh || !Sequence)
    {
        ResetToRefPose();
        return CurrentPose;
    }

    FReferenceSkeleton RefSkeleton;
    SkelMesh->GetRefSkeleton(RefSkeleton);

    if (bInBlend && PrevSequence) {
        TArray<FTransform> PoseA, PoseB;
        UAnimDataModel* DataModelA = PrevSequence->GetDataModel();
        UAnimDataModel* DataModelB = Sequence->GetDataModel();
        DataModelA->GetPoseAtTime(PrevTime, PoseA, RefSkeleton, PrevSequence->IsLooping());
        DataModelB->GetPoseAtTime(NextTime, PoseB, RefSkeleton, Sequence->IsLooping());

        // 보간
        float Alpha = BlendElapsed / BlendDuration;
        CurrentPose.SetNum(PoseA.Num());
        for (int i = 0; i < PoseA.Num(); ++i)
            CurrentPose[i].Blend(PoseA[i], PoseB[i], Alpha);
    }
    
    else {
        if (CurrentPose.Num() != RefSkeleton.GetRawBoneNum())
        {
            CurrentPose.SetNum(RefSkeleton.GetRawBoneNum());
        }

        UAnimDataModel* DataModel = Sequence->GetDataModel();
        const float SequenceLength = Sequence->GetPlayLength();

        if (!DataModel || SequenceLength < 0.f)
        {
            ResetToRefPose();
            return CurrentPose;
        }

        DataModel->GetPoseAtTime(CurrentTime, CurrentPose, RefSkeleton, Sequence->IsLooping());
    }
    return CurrentPose;

}

void UAnimInstance::ResetToRefPose()
{
    if (OwningComp && OwningComp->GetSkeletalMesh())
    {
        FReferenceSkeleton RefSkeleton;
        OwningComp->GetSkeletalMesh()->GetRefSkeleton(RefSkeleton);
        CurrentPose = RefSkeleton.RawRefBonePose;
    }
}
USkeletalMeshComponent* UAnimInstance::GetSkelMeshComponent()
{
    return Cast<USkeletalMeshComponent>(OwningComp);
}

void UAnimInstance::SetCurrentTime(float NewTime)
{
    if (Sequence)
    {
        CurrentTime = FMath::Clamp(NewTime,0.f,Sequence->GetPlayLength());
    }
    else
    {
        UE_LOG(ELogLevel::Warning, TEXT("SetCurrentTime: Sequence is NULL"));
    }
}

void UAnimInstance::SetCurrentSequence(UAnimSequenceBase* NewSeq, float NewTime)
{
    Sequence = NewSeq;
    NextTime = NewTime;
}

void UAnimInstance::StartTransition()
{
    // 전이 시작
    auto& Transition = AnimSM->PendingTransition;
    // 전이 시 PrevSequence가 직전 상태 Sequence인 경우만 가정
    PrevSequence = Transition->FromState->GetLinkAnimationSequence();
    PrevTime = CurrentTime;
    Sequence = Transition->ToState->GetLinkAnimationSequence();
    NextTime = 0.f;
    BlendDuration = Transition->Duration;
    BlendElapsed = 0.f;
    bInBlend = true;
    Transition->bIsBlending = false;
    AnimSM->PendingTransition = nullptr;
}

void UAnimInstance::UpdateBlendTime(float DeltaSeconds)
{
    BlendElapsed = FMath::Min(BlendElapsed + DeltaSeconds, BlendDuration);
    float BlendAlpha = BlendElapsed / BlendDuration;

    // blend 완료 시점
    if (BlendAlpha >= 1.f) {
        bInBlend = false;
        PrevSequence = nullptr;
        CurrentTime = NextTime;     // 다음 시퀀스로 전환된 시간 사용
    }
    else {
        // 전이 중에는 두 시퀀스 시간도 함게 갱신
        PrevTime += DeltaSeconds * (PrevSequence->GetRateScale());
        NextTime += DeltaSeconds * (Sequence->GetRateScale());
    }
}

void UAnimInstance::UpdateSingleAnimTime(float DeltaSeconds)
{
    if (Sequence) {
        CurrentTime += DeltaSeconds * Sequence->GetRateScale();

        // FIXING : 제거해도 되는 부분 확인하기
        if (UAnimSequenceBase* GetSequence = AnimSM->GetCurrentAnimationSequence())
        {
            if (Sequence != GetSequence)
            {
                CurrentTime = 0.0f;
            }

            Sequence = AnimSM->GetCurrentAnimationSequence();
            bPlaying = true;
        }
    }
}



void UAnimInstance::NativeInitializeAnimation()
{
}

void UAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    if (AnimSM == nullptr)
    {
        return;
    }
    
    AnimSM->ProcessState();
    
    if (UAnimSequenceBase* GetSequence = AnimSM->GetCurrentAnimationSequence())
    {
        if (Sequence != GetSequence)
        {
            CurrentTime = 0.0f;
        }
        
        Sequence = GetSequence;
        bPlaying = true; // 아마도 BeginPlay 시 호출해야할 듯?
    }
}

bool UAnimInstance::HandleNotify(const FAnimNotifyEvent& NotifyEvent)
{
    return false;
}

void UAnimInstance::TriggerSingleAnimNotify(const FAnimNotifyEvent& AnimNotifyEvent)
{
    if (HandleNotify(AnimNotifyEvent)) // 사용자가 AnimInstance단에서 오버라이딩한 경우 종료
    {
        return;
    }

    const float TriggerTime = AnimNotifyEvent.GetTriggerTime();
    const FName NotifyName = AnimNotifyEvent.NotifyName;

    if (OwningComp)
    {
        UE_LOG(ELogLevel::Display, TEXT("[Notify Triggered] Name: %s, TriggerTime: %.3f, OwningComp: VALID"), *NotifyName.ToString(), TriggerTime);

        ACharacter* Owner = Cast<ACharacter>(OwningComp->GetOwner());
        if (Owner)
        {
            UE_LOG(ELogLevel::Display, TEXT(" └ Owner: VALID (%s)"), *Owner->GetName());
            Owner->HandleAnimNotify(AnimNotifyEvent);
        }
        else
        {
            UE_LOG(ELogLevel::Warning, TEXT(" └ Owner: NULL"));
        }
    }
    else
    {
        UE_LOG(ELogLevel::Warning, TEXT("[Notify Triggered] Name: %s, TriggerTime: %.3f, OwningComp: NULL"), *NotifyName.ToString(), TriggerTime);
    }

}

void UAnimInstance::TriggerAnimNotifies(float DeltaSeconds)
{
    if (!Sequence || !OwningComp)
    {
        return;
    }

    float PrevTime = CurrentTime - DeltaSeconds * Sequence->GetRateScale();
    float CurrTime = CurrentTime;

    TArray<FAnimNotifyEvent> NotifyStateBeginEvent;
    Sequence->GetAnimNotifies(PrevTime, DeltaSeconds, Sequence->IsLooping(), NotifyStateBeginEvent);
    NotifyQueue.AddAnimNotifies(NotifyStateBeginEvent);

    // 새롭게 활성화된 Anim Notify State 배열
    TArray<FAnimNotifyEvent> NewActiveAnimNotifyState;
    NewActiveAnimNotifyState.Reserve(NotifyQueue.AnimNotifies.Num());

    for (FAnimNotifyEvent& NotifyEvent : NotifyStateBeginEvent)
    {
        /* Duration 구간 내에 지속적으로 Notified 되는 State 유형 */
        if (NotifyEvent.IsStateNotify())
        {
            // Begin 처리: 이전 프레임에 없던 경우
            if (!ActiveAnimNotifyState.Contains(NotifyEvent))
            {
                NotifyEvent.NotifyStateClass->NotifyBegin(OwningComp, NotifyEvent.GetDuration());
            }

            // Tick 처리: 이미 활성화 Notify Event 배열에 존재하는 경우
            NotifyEvent.NotifyStateClass->NotifyTick(OwningComp, DeltaSeconds);

            NewActiveAnimNotifyState.Add(NotifyEvent);
            //NotifyQueue.AddStateNotify(NotifyEvent);
        }
        else
        {
            TriggerSingleAnimNotify(NotifyEvent);
        }
    }

    // End 처리: 이전에는 있었는데 이번에는 없는 Notify
    for (FAnimNotifyEvent& OldActive : ActiveAnimNotifyState)
    {
        if (!NewActiveAnimNotifyState.Contains(OldActive))
        {
            if (OldActive.IsStateNotify())
            {
                OldActive.NotifyStateClass->NotifyEnd(OwningComp);
            }

            if (!OwningComp || this->bPlaying == false)
            {
                UE_LOG(ELogLevel::Warning, TEXT("▶ While Notify End, AnimationInstance has been destroyed or ended "));
                return;
            }
        }
    }

    // 현재 활성 상태 업데이트
    ActiveAnimNotifyState = std::move(NewActiveAnimNotifyState);
}

