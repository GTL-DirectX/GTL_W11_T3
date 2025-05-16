#include "ParticleEmitterInstances.h"

void FParticleDataContainer::Alloc(int32 InParticleDataNumBytes, int32 InParticleIndicesNumShorts)
{
    if (InParticleDataNumBytes > 0 && ParticleIndicesNumShorts >= 0
        && InParticleDataNumBytes % sizeof(uint16) == 0)
    {
        assert(InParticleDataNumBytes > 0 && ParticleIndicesNumShorts >= 0
            && InParticleDataNumBytes % sizeof(uint16) == 0); // we assume that the particle storage has reasonable alignment below);
    }

    ParticleDataNumBytes = InParticleDataNumBytes;
    ParticleIndicesNumShorts = InParticleIndicesNumShorts;

    MemBlockSize = ParticleDataNumBytes + ParticleIndicesNumShorts * sizeof(uint16);

    //ParticleData = (uint8*)FastParticleSmallBlockAlloc(MemBlockSize); // 메모리 할당 로직 정의 필요.
    ParticleIndices = (uint16*)(ParticleData + ParticleDataNumBytes);
}

void FParticleDataContainer::Free()
{
    if (ParticleData)
    {
        if (MemBlockSize > 0)
        {
            assert(MemBlockSize > 0);
        }
        
        //FastParticleSmallBlockFree(ParticleData, MemBlockSize); Free 해주는 로직.
    }
    MemBlockSize = 0;
    ParticleDataNumBytes = 0;
    ParticleIndicesNumShorts = 0;
    ParticleData = nullptr;
    ParticleIndices = nullptr;
}

void FParticleEmitterInstance::SpawnParticles(int32 Count, float StartTime, float Increment, const FVector& InitialLocation, const FVector& InitialVelocity, FParticleEventInstancePayload* EventPayload)
{
    for (int32 i = 0; i < Count; i++)
    {
        // Macro 추가 필요.
        DECLARE_PARTICLE_PTR(Particle, ParticleData + (ActiveParticles * ParticleStride));
        PreSpawn(Particle, InitialLocation, InitialVelocity);

        for (int32 ModuleIndex = 0; ModuleIndex < LODLevel->SpawnModules.Num(); ModuleIndex++)
        {
            
        }

        PostSpawn(Particle, Interp, SpawnTime);
    }
}

void FParticleEmitterInstance::KillParticle(int32 Index)
{
}

FDynamicEmitterDataBase::FDynamicEmitterDataBase(const UParticleModuleRequired* RequiredModule)
    : bSelected(false)
    , EmitterIndex(INDEX_NONE)
{
}

FDynamicSpriteEmitterReplayDataBase::FDynamicSpriteEmitterReplayDataBase()
    //: MaterialInterface(nullptr)
    : RequiredModule(nullptr)
    , NormalsSphereCenter(FVector::ZeroVector)
    , NormalsCylinderDirection(FVector::ZeroVector)
    , InvDeltaSeconds(0.0f)
    , MaxDrawCount(0)
    , OrbitModuleOffset(0)
    , DynamicParameterDataOffset(0)
    , LightDataOffset(0)
    , LightVolumetricScatteringIntensity(0)
    , CameraPayloadOffset(0)
    , SubUVDataOffset(0)
    , SubImages_Horizontal(1)
    , SubImages_Vertical(1)
    , bUseLocalSpace(false)
    , bLockAxis(false)
    , ScreenAlignment(0)
    , LockAxisFlag(0)
    , EmitterRenderMode(0)
    , EmitterNormalsMode(0)
    , PivotOffset(-0.5f, -0.5f)
    , bUseVelocityForMotionBlur(false)
    , bRemoveHMDRoll(false)
    , MinFacingCameraBlendDistance(0.f)
    , MaxFacingCameraBlendDistance(0.f)
{
}


FDynamicSpriteEmitterReplayDataBase::~FDynamicSpriteEmitterReplayDataBase()
{
    delete RequiredModule;
}

void* FDynamicEmitterDataBase::operator new(size_t AllocSize)
{
    //return FastParticleSmallBlockAlloc(AllocSize);
}

void FDynamicEmitterDataBase::operator delete(void* RawMemory, size_t AllocSize)
{
    //FastParticleSmallBlockFree(RawMemory, AllocSize);
}


/** FDynamicSpriteEmitterReplayDataBase Serialization */
void FDynamicSpriteEmitterReplayDataBase::Serialize(FArchive& Ar)
{
    // Call parent implementation
    FDynamicEmitterReplayDataBase::Serialize(Ar);

    Ar << ScreenAlignment;
    Ar << bUseLocalSpace;
    Ar << bLockAxis;
    Ar << LockAxisFlag;
    Ar << MaxDrawCount;

    int32 EmitterRenderModeInt = EmitterRenderMode;
    Ar << EmitterRenderModeInt;
    EmitterRenderMode = EmitterRenderModeInt;

    Ar << OrbitModuleOffset;
    Ar << DynamicParameterDataOffset;
    Ar << LightDataOffset;
    Ar << LightVolumetricScatteringIntensity;
    Ar << CameraPayloadOffset;

    Ar << EmitterNormalsMode;
    Ar << NormalsSphereCenter;
    Ar << NormalsCylinderDirection;

    //Ar << MaterialInterface;

    Ar << PivotOffset;

    Ar << bUseVelocityForMotionBlur;
    Ar << bRemoveHMDRoll;
    Ar << MinFacingCameraBlendDistance;
    Ar << MaxFacingCameraBlendDistance;
}
