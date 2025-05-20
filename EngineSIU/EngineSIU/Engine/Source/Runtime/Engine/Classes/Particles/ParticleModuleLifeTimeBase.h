#pragma once

#include "ParticleModule.h"

class UParticleModuleLifeTimeBase : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleLifeTimeBase, UParticleModule)

public:
    UParticleModuleLifeTimeBase() = default;

    virtual float GetMaxLifetime()
    {
        return 0.0f;
    }

    virtual float	GetLifetimeValue(FParticleEmitterInstance* Owner, float InTime, UObject* Data = NULL);

};
