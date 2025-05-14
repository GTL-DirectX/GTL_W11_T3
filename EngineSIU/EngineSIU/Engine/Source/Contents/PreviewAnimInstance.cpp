#include "PreviewAnimInstance.h"

#include "Animation/AnimationStateMachine.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Actor.h"
#include "UObject/ObjectFactory.h"

void UPreviewAnimInstance::NativeInitializeAnimation()
{
    UAnimInstance::NativeInitializeAnimation();

    UAnimNode_State* Idle = FObjectFactory::ConstructObject<UAnimNode_State>(this);
    Idle->Initialize(FName("Idle"), Anim1);
    Idle->GetLinkAnimationSequenceFunc = [this]() { return Anim1; };
    AnimSM->AddState(Idle);

    UAnimNode_State* Walk = FObjectFactory::ConstructObject<UAnimNode_State>(this);
    Walk->Initialize(FName("Walk "), Anim2);
    Walk->GetLinkAnimationSequenceFunc = [this]() { return Anim2; };
    AnimSM->AddState(Walk);

    AnimSM->AddTransition(Idle, Walk, [this]() { return this->bIdle_Walk; });
    AnimSM->AddTransition(Walk, Idle, [this]() { return !this->bIdle_Walk; });

    //AnimSM->SetState(FName("Idle"));
}

void UPreviewAnimInstance::UpdateAnimation(float DeltaSeconds)
{
    Super::UpdateAnimation(DeltaSeconds);
}
