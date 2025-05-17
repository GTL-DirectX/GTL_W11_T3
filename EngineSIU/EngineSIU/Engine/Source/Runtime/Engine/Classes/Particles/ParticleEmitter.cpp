#include "ParticleEmitter.h"

#include "ParticleHelper.h"

void UParticleEmitter::CacheEmitterModuleInfo()
{
    ParticleSize = sizeof(FBaseParticle);

    /*
    ........
    */
}

UParticleLODLevel* UParticleEmitter::GetCurrentLODLevel(FParticleEmitterInstance* Instance)
{
    return Instance->CurrentLODLevel;
}
