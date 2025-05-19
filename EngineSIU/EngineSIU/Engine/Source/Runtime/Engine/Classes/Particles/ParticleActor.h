#pragma once

#include "GameFramework/Actor.h"

class AParticleActor : public AActor
{
    DECLARE_CLASS(AParticleActor, AActor)

public:
    AParticleActor() = default;

    virtual void PostSpawnInitialize() override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void BeginPlay() override;

private:
    class UParticleSystemComponent* ParticleSystemComponent;

};
