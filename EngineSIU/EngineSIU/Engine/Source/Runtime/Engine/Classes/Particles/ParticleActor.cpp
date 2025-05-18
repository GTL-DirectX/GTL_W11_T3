#include "ParticleActor.h"

#include "ParticleSystemComponent.h"

void AParticleActor::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();

    ParticleSystemComponent = AddComponent<UParticleSystemComponent>("ParticleSystemComponent");
}

UObject* AParticleActor::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    NewActor->ParticleSystemComponent = Cast<UParticleSystemComponent>(ParticleSystemComponent->Duplicate(InOuter));


    return NewActor;
}
