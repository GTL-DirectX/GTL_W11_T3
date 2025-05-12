#include "AnimSequenceBase.h"
#include "AnimationRuntime.h"

UAnimSequenceBase::UAnimSequenceBase()
{
    RateScale = 1.0f;
}

bool UAnimSequenceBase::IsNotifyAvailable() const
{
    return (Notifies.Num() != 0) && (GetPlayLength() > 0.f);
}

/*
 * 새로운 프레임 내에 발생한 Notify를 추출하여 반환합니다
 */
void UAnimSequenceBase::GetAnimNotifies(
    const float& StartTime, const float& DeltaTime, const bool bAllowLooping, TArray<FAnimNotifyEvent>& OutActiveNotifies
) const
{
    OutActiveNotifies.Empty();

    if (!IsNotifyAvailable())
    {
        return;
    }

    bool const bPlayingBackwards = (DeltaTime < 0.f);
    float PreviousPosition = StartTime;
    float CurrentPosition = StartTime;
    float DesiredDeltaMove = DeltaTime;
    const float PlayLength = GetPlayLength();

    // 최대 루프 횟수 (무한 루프 방지용)
    // 짧은 애니메이션이 매우 큰 DeltaTime으로 여러 번 루프 재생되는 경우 처리 - 보호 로직
    // 2번째 조건 : 프레임 사이 시간 차가 너무 커서 한 프레임 동안 Animation 여러 번 반복
    // AnimNotify는 루프마다 트리거될 수 있으므로, [2, 1000] Clamp
    uint32 MaxLoopCount = 2;
    if (PlayLength > 0.0f && FMath::Abs(DeltaTime) > PlayLength)
    {
        MaxLoopCount = FMath::Clamp(static_cast<uint32>(DesiredDeltaMove / PlayLength), static_cast<uint32>(2), static_cast<uint32>(1000));
    }

    // 각 루프마다 Notify가 반복 호출될 수 있음
    for (uint32 i = 0; i < MaxLoopCount; ++i)
    {
        const ETypeAdvanceAnim AdvanceType = FAnimationRuntime::AdvanceTime(false, DesiredDeltaMove, CurrentPosition, PlayLength);
        GetAnimNotifiesFromDeltaPositions(Notifies, PreviousPosition, CurrentPosition, OutActiveNotifies);
        if (AdvanceType == ETAA_Finished)
        {
            break;
        }
        else if (AdvanceType == ETAA_Looped)
        {
            float ActualDeltaMove = (CurrentPosition - PreviousPosition);
            DesiredDeltaMove -= ActualDeltaMove;

            PreviousPosition = bPlayingBackwards ? PlayLength : 0.f;
            CurrentPosition = PreviousPosition;
        }
        else
        {
            break;
        }
    }
}

/*
 * 
 * Notify 범위 체크
 * 정방향: NotifyStartTime <= CurrentPosition && NotifyEndTime > PreviousPosition
 * 역방향: NotifyStartTime < PreviousPosition && NotifyEndTime >= CurrentPosition
 */
void UAnimSequenceBase::GetAnimNotifiesFromDeltaPositions(
    const TArray<FAnimNotifyEvent>& Notifies, float PreviousPosition, float CurrentPosition, TArray<FAnimNotifyEvent>& OutNotifies
) const
{
    const bool bPlayingBackwards = (CurrentPosition < PreviousPosition);

    for (const FAnimNotifyEvent& Notify : Notifies)
    {
        const float NotifyStartTime = Notify.GetTriggerTime();
        const float NotifyEndTime = Notify.GetEndTriggerTime();

        if (!bPlayingBackwards)
        {
            // 정방향: (이전시간, 현재시간] 걸치는 Notify 추출
            if (NotifyStartTime <= CurrentPosition && NotifyEndTime > PreviousPosition)
            {
                OutNotifies.Add(Notify);
            }
        }
        else
        {
            // 역방향: [현재시간, 이전시간) 걸치는 Notify 추출
            if (NotifyStartTime < PreviousPosition && NotifyEndTime >= CurrentPosition)
            {
                OutNotifies.Add(Notify);
            }
        }
    }
}






