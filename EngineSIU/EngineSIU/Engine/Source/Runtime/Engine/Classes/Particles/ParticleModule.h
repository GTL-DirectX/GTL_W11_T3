#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "ParticleHelper.h"
#include "Math/Color.h"

class UParticleModuleTypeDataBase;
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

    // ParticleModule 의 속성들 일단은 None 으로 설정
    UPROPERTY(EditAnywhere, bool, bSpawnModule)
    // UPROPERTY(EditAnywhere, uint8, bSpawnModule)
    /*uint8 bSpawnModule : 1;*/

    UPROPERTY(None, uint8, bUpdateModule)
    UPROPERTY(None, uint8, bFinalUpdateModule)
    UPROPERTY(None, uint8, bUpdateForGPUEmitter)
    UPROPERTY(None, uint8, bCurvesAsColor)
    UPROPERTY(None, uint8, b3DDrawMode)
    UPROPERTY(None, uint8, bSupported3DDrawMode)
    UPROPERTY(None, uint8, bEnabled)
    UPROPERTY(None, uint8, bEditable)
    UPROPERTY(None, FColor, EditorColor)

    virtual void	Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase);

    virtual void	Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime);

    virtual void	FinalUpdate(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime);

    virtual EModuleType	GetModuleType() const { return EPMT_General; }


    /* Number of bytes the module needs per "Particle" */
    virtual uint32 RequiredBytes(UParticleModuleTypeDataBase * TypeData);

    /* Number of bytes module needs per "Emitter Instance" */
    virtual uint32 RequiredBytesPerInstance();
    virtual uint32 PrepPerInstanceBlock(FParticleEmitterInstance* Owner, void* InstData);

};
