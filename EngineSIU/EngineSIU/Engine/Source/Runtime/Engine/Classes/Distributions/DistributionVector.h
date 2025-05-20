#pragma once
#include "HAL/PlatformType.h"
#include "Math/Vector.h"
#include "UObject/ObjectMacros.h"

class UObject;

struct FRawDistributionVector
{
    class UDistributionVector* Distribution;

    FRawDistributionVector()
        : Distribution(nullptr)
    {}

    FVector GetValue(float Time = 0.0f, UObject* Data = nullptr, int32 Extreme = 0);
};

class UDistributionVector : public UObject
{
    DECLARE_ABSTRACT_CLASS(UDistributionVector, UObject)
public:
    UDistributionVector() = default;
    virtual FVector GetValue(float Time, UObject* Data, int32 Extreme) const = 0;
};
