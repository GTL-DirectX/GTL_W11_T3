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
    /**
     * Set the transition to FName.
     */
    void AddTransition(FName FromStateName, FName ToStateName, const std::function<bool()>& Condition, float Duration = 0.2f);

    /**
     * Set the transition to UAnimNode_State.
     */
    void AddTransition(UAnimNode_State* FromState, UAnimNode_State* ToState, const std::function<bool()>& Condition, float Duration = 0.2f);

    void AddState(UAnimNode_State* State);
    
    void SetState(FName NewStateName);

    void SetStateInternal(uint32 NewState);

    void ProcessState();

    void ClearTransitions();

    void ClearStates();

    FAnimTransition& GetPendingTransition() const { return *PendingTransition; }
    void SetPendingTransition(FAnimTransition* NewTransition) { PendingTransition = NewTransition; }
    
    /**
     * 
     * OutFrom : 현재 재생되고 있는 애님 시퀀스
     * OutTo : 변경될 애님 시퀀스
     */
    void GetAnimationsForBlending(UAnimSequenceBase*& OutFrom, UAnimSequenceBase*& OutTo) const;
    
    FORCEINLINE uint32 GetCurrentState() const { return CurrentState; }
    
    FORCEINLINE bool GetTransitionState() const { return bTransitionState; }
    
    FORCEINLINE void SetTransitionState(bool NewState) { bTransitionState = NewState; }
    
    UAnimSequenceBase* GetCurrentAnimationSequence() const;

    FORCEINLINE TMap<uint32, UAnimNode_State*>& GetStateContainer() { return StateContainer; }

    void SetCurrentAnimationSequence(UAnimSequenceBase* NewAnim) { CurrentAnimationSequence = NewAnim; }

    FORCEINLINE TArray<FAnimTransition>& GetTransitions() { return Transitions; }

    /** State Container */
    TMap<uint32, UAnimNode_State*> StateContainer;
   
private:
 
    /** FName comparison index by state name */
    uint32 CurrentState;

    /** Current playing animation sequence */
    UAnimSequenceBase* CurrentAnimationSequence = nullptr;

    /** Anim waiting for transfer */
    UAnimSequenceBase* FromAnimationSequence = nullptr;

    /** Transition list */
    TArray<FAnimTransition> Transitions;

    /** Is pending transition */
    FAnimTransition* PendingTransition;
    
    

    
    /** true when a state transition occurs */
    bool bTransitionState = false;
};
