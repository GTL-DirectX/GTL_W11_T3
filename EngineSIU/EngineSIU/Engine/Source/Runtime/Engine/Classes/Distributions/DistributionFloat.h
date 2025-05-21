#pragma once
#include "HAL/PlatformType.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UDistributionFloat : public UObject
{
    DECLARE_ABSTRACT_CLASS(UDistributionFloat, UObject)
public:
    UDistributionFloat() = default;
    virtual float GetValue(float Time) const = 0;
    virtual ~UDistributionFloat() = default;
};

