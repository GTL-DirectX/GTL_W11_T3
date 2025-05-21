#pragma once

#include "ParticleModuleColorBase.h"

class UParticleModuleColor : public UParticleModuleColorBase
{
    DECLARE_CLASS(UParticleModuleColor, UParticleModuleColorBase)

public:
    UParticleModuleColor();
    
    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void PostInitProperties() override;

    virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase);
    virtual void Update(FParticleEmitterInstance* Owner, int32 Offset, float DeltaTime) override;



    UPROPERTY(EditAnywhere, FLinearColor, Color)

};
