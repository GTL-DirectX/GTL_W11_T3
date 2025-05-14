#pragma once
#pragma once
#include "Animation/AnimInstance.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"


class UPreviewAnimInstance : public UAnimInstance
{
    DECLARE_CLASS(UPreviewAnimInstance, UAnimInstance)

public:
    UPreviewAnimInstance() = default;

    virtual void NativeInitializeAnimation() override;
    virtual void UpdateAnimation(float DeltaSeconds) override;

public:
    /** Hard Coding */
    UAnimSequenceBase* Anim1 = nullptr;
    UAnimSequenceBase* Anim2 = nullptr;


    bool bIdle_Walk = false;

    
};
