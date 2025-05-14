#include "AnimNode_State.h"
#include "Animation/AnimSequence.h"

void UAnimNode_State::Initialize(FName InStateName, UAnimSequenceBase* InLinkAnimationSequence)
{
    StateName = InStateName;
    LinkAnimationSequence = InLinkAnimationSequence;
}

UAnimSequenceBase* UAnimNode_State::GetLinkAnimationSequence() const
{
    return LinkAnimationSequence ? LinkAnimationSequence : GetLinkAnimationSequenceFunc();
}
