#pragma once

#include "ParticleModuleLocationBase.h"

class UParticleModuleLocation : public UParticleModuleLocationBase
{
    DECLARE_CLASS(UParticleModuleLocation, UParticleModuleLocationBase)

public:
    UParticleModuleLocation() = default;
    virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;


    FVector StartLocation;

};
