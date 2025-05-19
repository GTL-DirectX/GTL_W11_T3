#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UParticleModule;

class UParticleLODLevel : public UObject
{
    DECLARE_CLASS(UParticleLODLevel, UObject)

public:
    UParticleLODLevel() = default;

    // Particle LOD Level
    int32 LODLevel;
    bool bEnabled;

    class UParticleModuleRequired* RequiredModule;
    // Particle Module
    TArray<UParticleModule*> Modules;
    class UParticleModuleTypeDataBase* TypeDataModule;
};
