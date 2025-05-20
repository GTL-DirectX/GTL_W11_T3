#pragma once
#include "DistributionVector.h"
#include "Math/Vector.h"

class UDistributionVectorConstant : public UDistributionVector
{

public:
    FVector Constant = FVector::ZeroVector;

    virtual FVector GetValue(float Time, UObject* Data = nullptr, int32 Extreme = 0) const override;
};
