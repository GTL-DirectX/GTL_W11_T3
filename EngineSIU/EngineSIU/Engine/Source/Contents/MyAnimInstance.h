#pragma once
#include "Animation/AnimInstance.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"


class UMyAnimInstance : public UAnimInstance
{
    DECLARE_CLASS(UMyAnimInstance, UAnimInstance)

public:
    UMyAnimInstance() = default;

    virtual void NativeInitializeAnimation() override;
    virtual void UpdateAnimation(float DeltaSeconds) override;

public:
    /** Hard Coding */
    UAnimSequenceBase* Anim1 = nullptr;
    UAnimSequenceBase* Anim2 = nullptr;


    bool bIsFly = false;
};
