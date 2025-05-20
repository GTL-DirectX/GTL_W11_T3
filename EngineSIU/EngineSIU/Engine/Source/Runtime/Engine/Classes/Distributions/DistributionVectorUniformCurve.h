#pragma once
#include "DistributionVector.h"
#include "Container/Array.h"
#include "Math/Vector.h"

struct FUniformCurveKeyVec3
{
    float Time;
    FVector Min;
    FVector Max;
};


class UDistributionVectorUniformCurve : public UDistributionVector
{
public:
    TArray<FUniformCurveKeyVec3> Keys;

    virtual FVector GetValue(float Time, UObject* Data, int32 Extreme) const override;

    static FVector RandomInRange(const FVector& Min, const FVector& Max);
};
