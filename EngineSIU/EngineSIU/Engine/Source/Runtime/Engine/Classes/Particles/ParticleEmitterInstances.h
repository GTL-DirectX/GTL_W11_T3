#pragma once

#include "HAL/PlatformType.h"
#include "ParticleHelper.h"
#include <UnrealEd/ImGuiWidget.h>

class UParticleEmitter;
class UParticleSystemComponent;
class UParticleLODLevel;

struct FParticleEmitterInstance
{
    UParticleEmitter* SpriteTemplate;

    // Owner
    UParticleSystemComponent* Component;

    int32 CurrentLODLevelIndex;
    UParticleLODLevel* CurrentLODLevel;

    // Collision Enable? // 추가구현 항목이지만 우선 추가.
    bool bEnableCollision;

    /** Pointer to the particle data array.                             */
    uint8* ParticleData;
    /** Pointer to the particle index array.                            */
    uint16* ParticleIndices;
    /** Pointer to the instance data array.                             */
    uint8* InstanceData;
    /** The size of the Instance data array.                            */
    int32 InstancePayloadSize;
    /** The offset to the particle data.                                */
    int32 PayloadOffset;
    /** The total size of a particle (in bytes).                        */
    int32 ParticleSize;
    /** The stride between particles in the ParticleData array.         */
    int32 ParticleStride;
    /** The number of particles currently active in the emitter.        */
    int32 ActiveParticles;
    /** Monotonically increasing counter. */
    uint32 ParticleCounter;
    /** The maximum number of active particles that can be held inthe particle data array. **/
    int32 MaxActiveParticles;
    /** The fraction of time left over from spawning.                   */
    float SpawnFraction;

    void SpawnParticles(int32 Count, float StartTime, float Increment, const FVector& InitialLocation, const FVector& InitialVelocity, struct FParticleEventInstancePayload* EventPayload);

    virtual float Spawn(float DeltaTime);
    virtual void PreSpawn(FBaseParticle* Particle, const FVector& InitialLocation, const FVector& InitialVelocity);
    virtual void PostSpawn(FBaseParticle* Particle, float InterpolationPercentage, float SpawnTime);

    virtual void KillParticles();
    void KillParticle(int32 Index);

};
