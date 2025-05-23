#pragma once

#include "ParticleModuleLocationBase.h"
#include "Distributions/DistributionVector.h"

class UParticleModuleLocation : public UParticleModuleLocationBase
{
    DECLARE_CLASS(UParticleModuleLocation, UParticleModuleLocationBase)

public:
    UParticleModuleLocation();
    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void PostInitProperties() override;
    virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;


    UPROPERTY(EditAnywhere, FRawDistributionVector, StartLocation)
};
