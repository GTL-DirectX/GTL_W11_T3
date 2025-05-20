#pragma once
#include "DistributionFloat.h"

class UDistributionFloatUniform : public UDistributionFloat
{
    DECLARE_CLASS(UDistributionFloatUniform, UDistributionFloat)
public:
    UDistributionFloatUniform() = default;
    float Min = 0.0f;
    float Max = 1.0f;

    virtual float GetValue(float /*Time*/) const override;
    
};
