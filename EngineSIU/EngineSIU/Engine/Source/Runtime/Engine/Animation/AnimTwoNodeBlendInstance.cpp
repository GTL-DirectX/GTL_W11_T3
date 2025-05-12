#include "AnimTwoNodeBlendInstance.h"
#include "Engine/Classes/Components/SkeletalMeshComponent.h"
#include "Classes/Components/Mesh/SkeletalMesh.h"
#include "Animation/AnimData/AnimDataModel.h"
void UAnimTwoNodeBlendInstance::SetAnimationAsset(UAnimSequenceBase* NewAnimToPlay, bool bResetTimeAndState, float InBlendTime)
{
    if (To.Sequence == NewAnimToPlay) {
        return; // 동일 시퀀스면 무시
    }

    // 현재 To를 From으로 이동
    From = To;
    To.Sequence = NewAnimToPlay;
    To.RateScale = NewAnimToPlay->GetRateScale();

    if (bResetTimeAndState) {
        // ? 이게 false인 경우 To는 시간 초기화 어떻게 함?
        From.CurrentTime = 0.0f;
        To.CurrentTime = 0.0f;
    }

    ElaspedBlendTime = 0.0f;
    BlendAlpha = 0.0f;
    BlendTime = InBlendTime;
    bIsBlending = true;

    SetPlaying(true);
}

void UAnimTwoNodeBlendInstance::UpdateAnimation(float DeltaSeconds)
{
    if (!IsPlaying() || !OwningComp || !OwningComp->GetSkeletalMesh()) {
        ResetToRefPose();
        return;
    }

    From.Advance(DeltaSeconds);
    To.Advance(DeltaSeconds);

    if (bIsBlending && BlendTime > 0.0f) {
        ElaspedBlendTime += DeltaSeconds;
        BlendAlpha = FMath::Clamp(ElaspedBlendTime / BlendTime, 0.0f, 1.0f);

        if (BlendAlpha >= 1.0f) {
            From = To;
            To = FBlendState();
            bIsBlending = false;
            BlendAlpha = 1.0f;
            ElaspedBlendTime = 0.0f;
            // TODO 블렌딩 끝날 시 델리게이트 호출
            //OnBlendComplete.ExcuteIfBound();
        }
    }
    else {
        BlendAlpha = 1.0f;
    }
}

const TArray<FTransform>& UAnimTwoNodeBlendInstance::EvaluateAnimation()
{
    USkeletalMesh* SkelMesh = OwningComp->GetSkeletalMesh();
    FReferenceSkeleton RefSkeleton;
    SkelMesh->GetRefSkeleton(RefSkeleton);

    const int32 NumBones = RefSkeleton.GetRawBoneNum();
    PoseA.SetNum(NumBones);
    PoseB.SetNum(NumBones);
    CurrentPose.SetNum(NumBones);

    if (From.IsValid()) {
        From.Sequence->GetDataModel()->GetPoseAtTime(From.CurrentTime, PoseA, RefSkeleton, From.Sequence->IsLooping());
    }
    else {
        PoseA = RefSkeleton.RawRefBonePose;
    }

    for (int32 i = 0; i < NumBones; ++i) {
        CurrentPose[i].SetTranslation(FMath::Lerp(PoseA[i].GetTranslation(), PoseB[i].GetTranslation(), BlendAlpha));
        CurrentPose[i].SetRotation(FQuat::Slerp(PoseA[i].GetRotation(), PoseB[i].GetRotation(), BlendAlpha));
        CurrentPose[i].SetScale3D(FMath::Lerp(PoseA[i].GetScale3D(), PoseB[i].GetScale3D(), BlendAlpha));
    }

    return CurrentPose;
}
