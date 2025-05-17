#pragma once

#include "ParticleEmitterInstances.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Container/Array.h"

class UParticleLODLevel;

enum class EEmitterRenderMode : int
{
    ERM_Normal,
    ERM_Point,
    ERM_Cross,
    ERM_LightsOnly,
    ERM_None,
    ERM_MAX,
};


class UParticleEmitter : public UObject
{
    DECLARE_CLASS(UParticleEmitter, UObject)

public:
    UParticleEmitter() = default;
    
    FName EmitterName;
    int32 ParticleSize;


    TArray<UParticleLODLevel*> LODLevels;

    void CacheEmitterModuleInfo();
    UParticleLODLevel* GetCurrentLODLevel(FParticleEmitterInstance* Instance);
};
