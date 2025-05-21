#pragma once

#include "ParticleModule.h"

class UParticleModuleVelocityBase : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleVelocityBase, UParticleModule)

public:
    UParticleModuleVelocityBase() = default;
    virtual UObject* Duplicate(UObject* InOuter) override;

    // 월드 공간 기준 속도 적용 여부
    UPROPERTY(EditAnywhere, bool, bInWorldSpace)

    // PSC의 Scale을 속도에 적용할지 여부
    UPROPERTY(EditAnywhere, bool, bApplyOwnerScale)

};
