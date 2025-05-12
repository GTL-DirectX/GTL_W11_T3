#include "AnimInstance.h"

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
    NativeInitializeAnimation();
}


void UAnimInstance::UpdateAnimation(float DeltaSeconds)
{
    if (!bPlaying || !Sequence || !OwningComp || !OwningComp->GetSkeletalMesh())
    {
        // 재생 중이 아니거나, 필요한 정보가 없으면 현재 OutPose를 변경하지 않거나 참조 포즈로 설정
        ResetToRefPose();
        return;
    }

    // 사용자 확장 영역 - 커스텀 변수 또는 입력 값 설정
    NativeUpdateAnimation(DeltaSeconds);
    TriggerAnimNotifies(DeltaSeconds);
    NotifyQueue.Reset();

    // 1. 애니메이션 시간 업데이트
    const float RateScale = Sequence->GetRateScale();
    CurrentTime += DeltaSeconds * RateScale;

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

void UAnimInstance::NativeInitializeAnimation()
{
}

void UAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
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

    for (const FAnimNotifyEvent& NotifyEvent : NotifyStateBeginEvent)
    {
        TriggerSingleAnimNotify(NotifyEvent);
    }
}

