#pragma once
#include "AnimNotifyState.h"

class UAnimNotifyState_SlowMotion : public UAnimNotifyState
{
    DECLARE_CLASS(UAnimNotifyState_SlowMotion, UAnimNotifyState);
public:

    UAnimNotifyState_SlowMotion() = default;
    virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, float Duration) override;
    virtual void NotifyTick(USkeletalMeshComponent* MeshComp, float DeltaTime) override;
    virtual void NotifyEnd(USkeletalMeshComponent* MeshComp) override;

private:
    float OriginalTimeDilation = 1.0f; // 기존 시간 배율 저장
    float SlowRate = 0.3f;
};

