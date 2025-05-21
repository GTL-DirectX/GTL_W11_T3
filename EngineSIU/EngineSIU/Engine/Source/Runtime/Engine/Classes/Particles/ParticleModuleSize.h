#pragma once

#include "ParticleModuleSizeBase.h"
#include "Distributions/DistributionVector.h"

class UParticleModuleSize : public UParticleModuleSizeBase
{
    DECLARE_CLASS(UParticleModuleSize, UParticleModuleSizeBase)

public:
    UParticleModuleSize();
    virtual UObject* Duplicate(UObject* InOuter) override;


    virtual void PostInitProperties() override;
    virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;
    void Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime);

    FRawDistributionVector StartSize;
    
};
