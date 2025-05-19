#pragma once

#include "ParticleModuleVelocityBase.h"

class UParticleModuleVelocity : public UParticleModuleVelocityBase
{
    DECLARE_CLASS(UParticleModuleVelocity, UParticleModuleVelocityBase)

public:
    UParticleModuleVelocity() = default;

    virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;

    FVector StartVelocity;
};
