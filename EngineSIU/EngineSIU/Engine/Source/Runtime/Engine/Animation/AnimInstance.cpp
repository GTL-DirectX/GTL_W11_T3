#include "AnimInstance.h"

#include "AnimSequenceBase.h"
#include "AnimData/AnimDataModel.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/Mesh/SkeletalMesh.h"
#include "Engine/Asset/SkeletalMeshAsset.h"
#include "Math/Transform.h"

UAnimInstance::UAnimInstance()
    : OwningComp(nullptr)
    , Sequence(nullptr)
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


    // 1. 애니메이션 시간 업데이트
    const float RateScale = Sequence->GetRateScale();
    CurrentTime += DeltaSeconds * RateScale;

    const float SequenceLength = Sequence->GetSequenceLength();
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
    const float SequenceLength = Sequence->GetSequenceLength();

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

void UAnimInstance::NativeInitializeAnimation()
{
}

void UAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
}

void UAnimInstance::TriggerAnimNotifies(float DeltaSeconds)
{
    // TODO: 현재 시간과 이전 시간을 비교하여 Notify 트리거
}

