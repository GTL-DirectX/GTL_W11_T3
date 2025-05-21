#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UParticleModuleRequired;
class UParticleModuleSpawn;
class UParticleModuleTypeDataBase;
class UParticleModule;

class UParticleLODLevel : public UObject
{
    DECLARE_CLASS(UParticleLODLevel, UObject)

public:
    UParticleLODLevel() = default;
    virtual void PostInitProperties() override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void UpdateModuleLists();

    void AddModule(UClass* ModuleClass);
    void AddModule(UParticleModule* Module);

    void RemoveModule(UParticleModule* Module);

    // Particle LOD Level
    int32 LODLevel;
    bool bEnabled;
    int32 PeakActiveParticles;

    TArray<UParticleModule*> Modules;
    /* 파티클이 생성될 떄 단 한번만 호출 : Location, Size, Velocity, Color*/
    TArray<UParticleModule*> SpawnModules;
    /* 파티클이 살아있는 동안 매 프레임마다 호출 : VelocityOverLife, Acceleration, Drag.. */
    TArray<UParticleModule*> UpdateModules;

    UParticleModuleTypeDataBase* TypeDataModule;

    UParticleModuleRequired* RequiredModule;
    UParticleModuleSpawn* SpawnModule;

    bool bSpriteTypeData = true;
   
};
