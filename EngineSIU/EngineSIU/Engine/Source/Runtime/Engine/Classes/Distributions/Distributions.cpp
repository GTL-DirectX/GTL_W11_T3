#include "DistributionTypes.h"
#include "DistributionFloat.h"
#include "DistributionFloatUniform.h"
#include "DistributionFloatConstant.h"
#include "DistributionFloatUniformCurve.h"
#include "DistributionVector.h"
#include "DistributionVectorConstant.h"
#include "DistributionVectorUniform.h"
#include "DistributionVectorUniformCurve.h"
#include "Math/MathUtility.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"

float FRawDistributionFloat::GetValue(float Time, UObject* Data, int32 Extreme) const
{
    return Distribution ? Distribution->GetValue(Time) : 0.0f;
}

UObject* UDistributionFloatUniform::Duplicate(UObject* InOuter)
{
    UDistributionFloatUniform* NewObj = Cast<UDistributionFloatUniform>(Super::Duplicate(InOuter));
    if (NewObj)
    {
        NewObj->Min = Min;
        NewObj->Max = Max;
    }
    return NewObj;
}

float UDistributionFloatUniform::GetValue(float /*Time*/) const
{
    return FMath::FRandRange(Min, Max);
}

float UDistributionFloatConstant::GetValue(float /*Time*/) const
{
    return Constant;
}

float UDistributionFloatUniformCurve::GetValue(float F) const
{
    if (Keys.IsEmpty())
    {
        return 0.0f;
    }

    // 범위 밖일 경우 첫/마지막 키값 사용
    if (F <= Keys.First().Time)
    {
        return FMath::FRandRange(Keys.First().Min, Keys.First().Max);
    }
    if (F >= Keys.Last().Time)
    {
        return FMath::FRandRange(Keys.Last().Min, Keys.Last().Max);
    }

    // F 사이에 있는 키프레임 쌍 찾기
    for (size_t i = 1; i < Keys.Num(); ++i)
    {
        const FUniformCurveKey& Prev = Keys[i - 1];
        const FUniformCurveKey& Next = Keys[i];

        if (F >= Prev.Time && F <= Next.Time)
        {
            float Alpha = (F - Prev.Time) / (Next.Time - Prev.Time);
            float Min = FMath::Lerp(Prev.Min, Next.Min, Alpha);
            float Max = FMath::Lerp(Prev.Max, Next.Max, Alpha);
            return FMath::FRandRange(Min, Max);
        }
    }

    return 0.0f; // 예외처리, 웬만해선 여기까지 안 옴
}

FVector FRawDistributionVector::GetValue(float Time, UObject* Data, int32 Extreme)
{
    if (!Distribution)
    {
        return FVector::ZeroVector;
    }

    return Distribution->GetValue(Time, Data, Extreme); // FRandomStream 없이 nullptr
}

FVector UDistributionVectorConstant::GetValue(float Time, UObject* Data, int32 Extreme) const
{
    return Constant;
}

UObject* UDistributionVectorUniform::Duplicate(UObject* InOuter)
{
    UDistributionVectorUniform* NewObj = Cast<UDistributionVectorUniform>(Super::Duplicate(InOuter));
    if (NewObj)
    {
        NewObj->Min = Min;
        NewObj->Max = Max;
    }
    return NewObj;
}

FVector UDistributionVectorUniform::GetValue(float Time, UObject* Data, int32 Extreme) const
{
    return FVector(
        FMath::FRandRange(Min.X, Max.X),
        FMath::FRandRange(Min.Y, Max.Y),
        FMath::FRandRange(Min.Z, Max.Z)
    );
}

FVector UDistributionVectorUniformCurve::GetValue(float F, UObject* Data, int32 Extreme) const
{
    if (Keys.IsEmpty())
    {
        return FVector::ZeroVector;
    }

    if (F <= Keys[0].Time)
    {
        return RandomInRange(Keys[0].Min, Keys[0].Max);
    }
    if (F >= Keys.Last().Time)
    {
        return RandomInRange(Keys.Last().Min, Keys.Last().Max);
    }

    for (int i = 1; i < Keys.Num(); ++i)
    {
        const auto& Prev = Keys[i - 1];
        const auto& Next = Keys[i];

        if (F >= Prev.Time && F <= Next.Time)
        {
            float Alpha = (F - Prev.Time) / (Next.Time - Prev.Time);
            FVector Min = FMath::Lerp(Prev.Min, Next.Min, Alpha);
            FVector Max = FMath::Lerp(Prev.Max, Next.Max, Alpha);
            return RandomInRange(Min, Max);
        }
    }

    return FVector::ZeroVector;
}

FVector UDistributionVectorUniformCurve::RandomInRange(const FVector& Min, const FVector& Max)
{
    return FVector(
        FMath::FRandRange(Min.X, Max.X),
        FMath::FRandRange(Min.Y, Max.Y),
        FMath::FRandRange(Min.Z, Max.Z)
    );
}
