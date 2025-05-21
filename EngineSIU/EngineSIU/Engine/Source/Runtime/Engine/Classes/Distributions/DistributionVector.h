#pragma once
#include "HAL/PlatformType.h"
#include "UObject/ObjectMacros.h"

class UObject;

class UDistributionVector : public UObject
{
    DECLARE_ABSTRACT_CLASS(UDistributionVector, UObject)
public:
    UDistributionVector() = default;
    virtual FVector GetValue(float Time, UObject* Data, int32 Extreme) const = 0;
};
