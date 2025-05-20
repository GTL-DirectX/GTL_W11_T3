#pragma once
#include "Math/Vector.h"
#include "DistributionVector.h"


class UDistributionVectorUniform : public UDistributionVector
{
    DECLARE_CLASS(UDistributionVectorUniform, UDistributionVector)
public:
    UDistributionVectorUniform() = default;

    FVector Min = FVector::ZeroVector;
    FVector Max = FVector::ZeroVector;

    virtual FVector GetValue(float Time, UObject* Data, int32 Extreme) const override;
};
