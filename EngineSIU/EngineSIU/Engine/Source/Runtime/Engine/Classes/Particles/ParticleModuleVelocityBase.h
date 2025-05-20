#pragma once

#include "ParticleModule.h"

class UParticleModuleVelocityBase : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleVelocityBase, UParticleModule)

public:
    UParticleModuleVelocityBase() = default;

    // 월드 공간 기준 속도 적용 여부
    uint32 bInWorldSpace : 1 = false;

    // PSC의 Scale을 속도에 적용할지 여부
    uint32 bApplyOwnerScale : 1 = false; 

};
