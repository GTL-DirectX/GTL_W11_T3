#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UParticleModuleRequired;
class UParticleModuleTypeDataBase;
class UParticleModule;

class UParticleLODLevel : public UObject
{
    DECLARE_CLASS(UParticleLODLevel, UObject)

public:
    UParticleLODLevel() = default;
    virtual void UpdateModuleLists();

    // Particle LOD Level
    int32 LODLevel;
    bool bEnabled;
    int32 PeakActiveParticles;


    // Particle Module
    TArray<UParticleModule*> SpawnModules;
    TArray<UParticleModule*> Modules;
    TArray<UParticleModule*> UpdateModules;

    UParticleModuleTypeDataBase* TypeDataModule;
    UParticleModuleRequired* RequiredModule;
   
    

};
