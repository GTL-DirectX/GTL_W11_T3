#pragma once
#include "AnimNode_State.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UAnimSequenceBase;

struct FAnimTransition
{
    UAnimNode_State* FromState;
    UAnimNode_State* ToState;

    /** Transition */
    int8 PriorityOrder = 1;
    std::function<bool()> Condition;

    /** Blend Setting */
    float Duration = 0.2f;

    /** Transition condition */
    bool CanTransition() const
    {
        return Condition && Condition();
    }
};


class UAnimationStateMachine : public UObject
{
    DECLARE_CLASS(UAnimationStateMachine, UObject)

public:
    UAnimationStateMachine() = default;

public:
    void AddTransition(UAnimNode_State* FromState, UAnimNode_State* ToState, const std::function<bool()>& Condition, float Duration = 0.2f);

    void AddState(UAnimNode_State* State);
    
    void SetState(FName NewStateName);

    void SetStateInternal(uint32 NewState);

    void ProcessState();
    
    FORCEINLINE uint32 GetCurrentState() const { return CurrentState; }
    UAnimSequenceBase* GetCurrentAnimationSequence() const;
    
private:
 
    /** FName comparison index by state name */
    uint32 CurrentState;

    /** Current playing animation sequence */
    UAnimSequenceBase* CurrentAnimationSequence = nullptr;

    /** Transition list */
    TArray<FAnimTransition> Transitions;

    /** State Container */
    TMap<uint32, UAnimNode_State*> StateContainer;
};
