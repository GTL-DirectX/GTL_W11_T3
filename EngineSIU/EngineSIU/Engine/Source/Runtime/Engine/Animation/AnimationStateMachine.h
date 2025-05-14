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

    void SetState(FName NewStateName);

    void SetStateInternal(uint32 NewState);

    void ProcessState();

    void ClearTransitions();

    /**
     * 
     * OutFrom : 현재 재생되고 있는 애님 시퀀스
     * OutTo : 변경될 애님 시퀀스
     */
    void GetAnimationsForPending(UAnimSequenceBase*& OutFrom, UAnimSequenceBase*& OutTo);
    
    FORCEINLINE uint32 GetCurrentState() const { return CurrentState; }
    FORCEINLINE bool GetTransitionState() const { return bTransitionState; }
    
    UAnimSequenceBase* GetCurrentAnimationSequence() const;
    
private:
 
    /** FName comparison index by state name */
    uint32 CurrentState;

    /** Current playing animation sequence */
    UAnimSequenceBase* CurrentAnimationSequence = nullptr;

    /** Anim waiting for transfer */
    UAnimSequenceBase* FromAnimationSequence = nullptr;

    /** Transition list */
    TArray<FAnimTransition> Transitions;
    
    /** true when a state transition occurs */
    bool bTransitionState = false;
};
