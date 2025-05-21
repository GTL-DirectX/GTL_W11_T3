#include "ParticleModuleVelocityBase.h"
#include "UObject/Casts.h"

UObject* UParticleModuleVelocityBase::Duplicate(UObject* InOuter)
{
    ThisClass* NewModule = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewModule->bInWorldSpace = bInWorldSpace;
    NewModule->bApplyOwnerScale = bApplyOwnerScale;
    return NewModule;
}
