#pragma once
#include "HAL/PlatformType.h"
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

enum class EDistributionType : uint8
{
    Const = 0,
    ConstCurve,
    Uniform,
    UniformCurve,
    Parameter,
    NONE
};


struct FRawDistributionFloat 
{
    EDistributionType Mode = EDistributionType::Uniform;
private:
    float MinValue;
    float MaxValue;

public:
    class UDistributionFloat* Distribution;

    FRawDistributionFloat()
        : MinValue(0)
        , MaxValue(0)
        , Distribution(nullptr)
    {
    }
    float GetValue(float Time = 0.0f, UObject* Data = nullptr, int32 Extreme = 0) const;

};

class UDistributionFloat : public UObject
{
    DECLARE_ABSTRACT_CLASS(UDistributionFloat, UObject)
public:
    UDistributionFloat() = default;
    virtual float GetValue(float Time) const = 0;
    virtual ~UDistributionFloat() = default;
};

