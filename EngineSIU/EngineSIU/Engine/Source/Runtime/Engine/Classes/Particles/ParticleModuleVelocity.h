#pragma once

#include "ParticleModuleVelocityBase.h"
#include "Distributions/DistributionFloat.h"
#include "Distributions/DistributionVector.h"

class UParticleModuleVelocity : public UParticleModuleVelocityBase
{
    DECLARE_CLASS(UParticleModuleVelocity, UParticleModuleVelocityBase)

public:
    UParticleModuleVelocity();
    virtual void PostInitProperties() override;

    virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;

    /* X, Y, Z 방향으로 초기 속도 분포 */
    FRawDistributionVector StartVelocity;

    /* Emitter 중심에서 바깥방향 방사형으로 퍼지는 추가 속도 */
    FRawDistributionFloat StartVelocityRadial;
};
