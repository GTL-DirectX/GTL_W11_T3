#pragma once

#include "Math/Vector.h"

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


struct FRawDistributionVector
{
    class UDistributionVector* Distribution;

    FRawDistributionVector()
        : Distribution(nullptr)
    {}

    FVector GetValue(float Time = 0.0f, UObject* Data = nullptr, int32 Extreme = 0);
};
