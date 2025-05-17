#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "ParticleHelper.h"
#include "Math/Color.h"

struct FParticleEmitterInstance;

enum EModuleType : int
{
    /** General - all emitter types can use it			*/
    EPMT_General,
    /** TypeData - TypeData modules						*/
    EPMT_TypeData,
    /** Beam - only applied to beam emitters			*/
    EPMT_Beam,
    /** Trail - only applied to trail emitters			*/
    EPMT_Trail,
    /** Spawn - all emitter types REQUIRE it			*/
    EPMT_Spawn,
    /** Required - all emitter types REQUIRE it			*/
    EPMT_Required,
    /** Event - event related modules					*/
    EPMT_Event,
    /** Light related modules							*/
    EPMT_Light,
    /** SubUV related modules							*/
    EPMT_SubUV,
    EPMT_MAX,
};


class UParticleModule : public UObject
{
    DECLARE_CLASS(UParticleModule, UObject)
    
public:
    UParticleModule() = default;

public:

    uint8 bSpawnModule : 1;
    uint8 bUpdateModule : 1;
    uint8 bFinalUpdateModule : 1;
    uint8 bUpdateForGPUEmitter : 1;
    uint8 bCurvesAsColor : 1;
    uint8 b3DDrawMode : 1;
    uint8 bSupported3DDrawMode : 1;
    uint8 bEnabled : 1;
    uint8 bEditable : 1;
    FColor EditorColor;

    /**
     *	Called on a particle that is freshly spawned by the emitter.
     *
     *	@param	Owner		The FParticleEmitterInstance that spawned the particle.
     *	@param	Offset		The modules offset into the data payload of the particle.
     *	@param	SpawnTime	The time of the spawn.
     */
    virtual void	Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase);
    /**
     *	Called on a particle that is being updated by its emitter.
     *
     *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
     *	@param	Offset		The modules offset into the data payload of the particle.
     *	@param	DeltaTime	The time since the last update.
     */
    virtual void	Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime);
    /**
     *	Called on an emitter when all other update operations have taken place
     *	INCLUDING bounding box cacluations!
     *
     *	@param	Owner		The FParticleEmitterInstance that 'owns' the particle.
     *	@param	Offset		The modules offset into the data payload of the particle.
     *	@param	DeltaTime	The time since the last update.
     */
    virtual void	FinalUpdate(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime);

    virtual EModuleType	GetModuleType() const { return EPMT_General; }


};
