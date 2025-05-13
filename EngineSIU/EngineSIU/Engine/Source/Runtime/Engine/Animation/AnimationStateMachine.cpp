#include "AnimationStateMachine.h"

#include "AnimSequenceBase.h"

void UAnimationStateMachine::AddTransition(UAnimNode_State* FromState, UAnimNode_State* ToState, const std::function<bool()>& Condition, float Duration)
{
    FAnimTransition NewTransition;
    NewTransition.FromState = FromState;
    NewTransition.ToState = ToState;
    NewTransition.Condition = Condition;
    NewTransition.Duration = Duration;

    Transitions.Add(NewTransition);
}

void UAnimationStateMachine::SetStateInternal(uint32 NewState)
{
    CurrentState = NewState;
}

void UAnimationStateMachine::SetState(FName NewStateName)
{
    uint32 ComparisonIndex = NewStateName.GetComparisonIndex();

    if (ComparisonIndex == 0)
    {
        return;
    }

    SetStateInternal(ComparisonIndex);
}

void UAnimationStateMachine::ProcessState()
{
    for (const auto& Transition : Transitions)
    {
        if (Transition.FromState->GetStateName() == CurrentState && Transition.Condition())
        {
            SetStateInternal(Transition.ToState->GetStateName());
            CurrentAnimationSequence = Transition.ToState->GetLinkAnimationSequence();
            break;
        }
    }
}

UAnimSequenceBase* UAnimationStateMachine::GetCurrentAnimationSequence() const
{
    return CurrentAnimationSequence;
}
