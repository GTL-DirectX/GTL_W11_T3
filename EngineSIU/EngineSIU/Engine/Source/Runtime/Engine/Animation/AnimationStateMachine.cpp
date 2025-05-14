#include "AnimationStateMachine.h"

#include "AnimSequenceBase.h"

void UAnimationStateMachine::AddTransition(FName FromStateName, FName ToStateName, const std::function<bool()>& Condition, float Duration)
{
    FAnimTransition T;
    T.FromState = StateContainer[FromStateName.GetComparisonIndex()];
    T.ToState = StateContainer[ToStateName.GetComparisonIndex()];
    T.Condition = Condition;
    T.Duration = Duration;

    Transitions.Add(T);
}

void UAnimationStateMachine::AddTransition(UAnimNode_State* FromState, UAnimNode_State* ToState, const std::function<bool()>& Condition, float Duration)
{
    FAnimTransition NewTransition;
    NewTransition.FromState = FromState;
    NewTransition.ToState = ToState;
    NewTransition.Condition = Condition;
    NewTransition.Duration = Duration;

    Transitions.Add(NewTransition);
}

void UAnimationStateMachine::AddState(UAnimNode_State* State)
{
    StateContainer[State->GetStateName()] = State;
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
    for (auto& Transition : Transitions)
    {
        if (Transition.FromState->GetStateName() == CurrentState
            && Transition.ToState
            && Transition.CanTransition()
            && !bTransitionState)
        {
            bTransitionState = true;
            PendingTransition = &Transition;
            
            SetStateInternal(Transition.ToState->GetStateName());

            CurrentAnimationSequence = Transition.ToState->GetLinkAnimationSequence();
            break;
        }
    }
}

void UAnimationStateMachine::ClearTransitions()
{
    Transitions.Empty();
}

void UAnimationStateMachine::ClearStates()
{
    CurrentAnimationSequence = nullptr;
    FromAnimationSequence = nullptr;
    CurrentState = NAME_None;
    StateContainer.Empty();
}

void UAnimationStateMachine::GetAnimationsForBlending(UAnimSequenceBase*& OutFrom, UAnimSequenceBase*& OutTo) const
{
    if (!bTransitionState)
    {
        OutFrom = nullptr;
        OutTo = nullptr;
        return;
    }
    
    OutFrom = FromAnimationSequence;
    OutTo = CurrentAnimationSequence;
}

UAnimSequenceBase* UAnimationStateMachine::GetCurrentAnimationSequence() const
{
    return CurrentAnimationSequence;
}
