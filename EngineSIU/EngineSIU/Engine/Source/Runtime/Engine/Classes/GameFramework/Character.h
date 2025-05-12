#pragma once
#include "Pawn.h"

class UCapsuleComponent;
class USkeletalMeshComponent;

struct FAnimNotifyEvent;
class ACharacter : public APawn
{
    DECLARE_CLASS(ACharacter, APawn);
public:
    ACharacter();
    virtual void PostSpawnInitialize() override;
    void HandleAnimNotify(const FAnimNotifyEvent& NotifyEvent) const;

private:
    USkeletalMeshComponent* Mesh;
    UCapsuleComponent* CapsuleComponent;
};
