#include "AnimTypes.h"
#include "AnimNotifies/AnimNotifyState.h"

float FAnimNotifyEvent::GetDuration() const
{
    return Duration;
}

float FAnimNotifyEvent::GetTriggerTime() const
{
    return TriggerTime + TriggerTimeOffset; // TriggerTime : [0, SequenceLength]
}

float FAnimNotifyEvent::GetEndTriggerTime() const
{
    if (!NotifyStateClass && (EndTriggerTimeOffset != 0.f))
    {
        UE_LOG(ELogLevel::Warning, TEXT("Anim Notify %s is non state, but has an EndTriggerTimeOffset %f!"), *NotifyName.ToString(), EndTriggerTimeOffset);
    }

    /* Notify State인 경우 Duration까지 고려하여 반환 */
    return NotifyStateClass ? (GetTriggerTime() + Duration + EndTriggerTimeOffset) : GetTriggerTime();
}

bool FAnimNotifyEvent::IsStateNotify() const
{
    return NotifyStateClass != nullptr;
}

bool operator==(const FAnimNotifyEvent& Lhs, const FAnimNotifyEvent& Rhs)
{
    return Lhs.NotifyName == Rhs.NotifyName &&
        Lhs.TriggerTime == Rhs.TriggerTime &&
        Lhs.Duration == Rhs.Duration &&
        Lhs.EndTriggerTimeOffset == Rhs.EndTriggerTimeOffset &&
        Lhs.TriggerTimeOffset == Rhs.TriggerTimeOffset &&
        Lhs.NotifyStateClass == Rhs.NotifyStateClass;
}

bool operator!=(const FAnimNotifyEvent& Lhs, const FAnimNotifyEvent& Rhs)
{
    return !(Lhs == Rhs);
}
