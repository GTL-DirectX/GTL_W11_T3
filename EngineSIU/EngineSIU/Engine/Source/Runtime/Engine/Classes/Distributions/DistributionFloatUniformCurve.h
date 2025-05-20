#pragma once
#include "DistributionFloat.h"
#include "Container/Array.h"


struct FUniformCurveKey
{
    float Time;
    float Min;
    float Max;
};

class UDistributionFloatUniformCurve : public UDistributionFloat
{
    DECLARE_CLASS(UDistributionFloatUniformCurve, UDistributionFloat)
public:
    TArray<FUniformCurveKey> Keys;
    UDistributionFloatUniformCurve()
    {
        // 간략한 예시로 구현하였음, 초기화는 다음과 같이 이뤄짐
        // Keys.Add({0.0f, 10.0f, 20.0f});
        // Keys.Add({1.0f, 30.0f, 50.0f});
    }

    virtual float GetValue(float F) const override;
};
