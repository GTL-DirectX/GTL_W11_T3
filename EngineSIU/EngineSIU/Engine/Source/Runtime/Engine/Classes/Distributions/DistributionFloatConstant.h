#pragma once
#include "DistributionFloat.h"

class UDistributionFloatConstant : public UDistributionFloat
{
    DECLARE_CLASS(UDistributionFloatConstant, UDistributionFloat)
public:
    UDistributionFloatConstant() = default;
    
    float Constant = 0.0f;

    virtual float GetValue(float /*Time*/) const override;
};
