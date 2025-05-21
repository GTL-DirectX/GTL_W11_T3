#pragma once

#include "ParticleModuleSizeBase.h"
#include "Distributions/DistributionVector.h"

class UParticleModuleSize : public UParticleModuleSizeBase
{
    DECLARE_CLASS(UParticleModuleSize, UParticleModuleSizeBase)

public:
    UParticleModuleSize();

    virtual void PostInitProperties() override;
    virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;

    FRawDistributionVector StartSize;
};
