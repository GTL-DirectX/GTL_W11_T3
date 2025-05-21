#pragma once

#include "ParticleModuleLifeTimeBase.h"
#include "Distributions/DistributionFloat.h"
#include "Distributions/DistributionVector.h"

class UParticleModuleLifeTime : public UParticleModuleLifeTimeBase
{
    DECLARE_CLASS(UParticleModuleLifeTime, UParticleModuleLifeTimeBase)

public:
    UParticleModuleLifeTime() = default;

    UPROPERTY(EditAnywhere, FRawDistributionFloat, LifeTime)

	virtual void PostInitProperties() override;
    virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase);


    
};
