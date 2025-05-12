#pragma once
#include "Container/Array.h"
#include "Math/Quat.h"
#include "Math/Vector.h"
#include "UObject/ObjectMacros.h"

/* [참고] UAnimNotify: 한 프레임에서 한 번만 발생하는 이벤트 */ 
struct FAnimNotifyEvent
{
    float TriggerTime;  // 발생 시간
    float Duration;     // 지속 시간 (0이면 단발성) = EndTriggerTime - TriggerTime
    FName NotifyName;   

    float TriggerTimeOffset = 0.f;        // Notify가 시작되도록 보정된 시간 오프셋
    float EndTriggerTimeOffset = 0.f;     // Notify 종료 보정을 위한 시간 오프셋

    float GetDuration() const;
    float GetTriggerTime() const;
    float GetEndTriggerTime() const;
};

/**
* [본 하나에 대한 전체 트랙 : T, R, S, Curve 묶음]
* 하나의 트랙에 대한 원시 키프레임 데이터입니다. 각 배열은 NumFrames 개의 요소 또는 1개의 요소를 포함합니다.
* 모든 키가 동일한 경우, 간단한 압축 방식으로 모든 키를 하나의 키로 줄여 전체 시퀀스에서 일정하게 유지됩니다.
*/
struct FRawAnimSequenceTrack
{
    TArray<FVector> PosKeys;
    TArray<FQuat> RotKeys;
    TArray<FVector> ScaleKeys;

    friend FArchive& operator<<(FArchive& Ar, FRawAnimSequenceTrack& T)
    {
        Ar << T.PosKeys;
        Ar << T.RotKeys;
        Ar << T.ScaleKeys;
        return Ar;
    }

    bool ContainsNaN() const
    {
        bool bContainsNaN = false;

        auto CheckForNan = [&bContainsNaN](const auto& Keys) -> bool
            {
                if (!bContainsNaN)
                {
                    for (const auto& Key : Keys)
                    {
                        if (Key.ContainsNaN()) 
                        {
                            return true;
                        }
                    }

                    return false;
                }

                return true;
            };

        bContainsNaN = CheckForNan(PosKeys);
        bContainsNaN = CheckForNan(RotKeys);
        bContainsNaN = CheckForNan(ScaleKeys);

        return bContainsNaN;
    }

    static constexpr uint32 SingleKeySize = sizeof(FVector) + sizeof(FQuat) + sizeof(FVector);
};


struct FBoneAnimationTrack
{
    FRawAnimSequenceTrack InternalTrackData;
    int32 BoneTreeIndex = INDEX_NONE;
    FName Name;
};
