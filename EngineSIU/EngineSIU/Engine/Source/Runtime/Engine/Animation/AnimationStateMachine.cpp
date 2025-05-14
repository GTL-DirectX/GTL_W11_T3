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
        if (Transition.FromState->GetStateName() == CurrentState && Transition.CanTransition() && !bTransitionState)
        {
            SetStateInternal(Transition.ToState->GetStateName());
            CurrentAnimationSequence = Transition.ToState->GetLinkAnimationSequence();
            FromAnimationSequence = Transition.FromState->GetLinkAnimationSequence();
            bTransitionState = true;
            break;
        }
    }
}

void UAnimationStateMachine::ClearTransitions()
{
    Transitions.Empty();
}

void UAnimationStateMachine::GetAnimationsForPending(UAnimSequenceBase*& OutFrom, UAnimSequenceBase*& OutTo)
{
    if (!bTransitionState)
    {
        OutFrom = nullptr;
        OutTo = nullptr;
        return;
    }
    
    OutFrom = FromAnimationSequence;
    OutTo = CurrentAnimationSequence;
    
    bTransitionState = false;
}

UAnimSequenceBase* UAnimationStateMachine::GetCurrentAnimationSequence() const
{
    return CurrentAnimationSequence;
}
