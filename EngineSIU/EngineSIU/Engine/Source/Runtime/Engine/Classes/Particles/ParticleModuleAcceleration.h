#pragma once

#include "ParticleModule.h"
#include "Distributions/DistributionVector.h"
#include "UObject/ObjectMacros.h"

class UParticleModuleAcceleration : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleAcceleration, UObject)
public:
    UParticleModuleAcceleration();


    virtual void PostInitProperties() override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;


    FRawDistributionVector GravityAccel;
};
