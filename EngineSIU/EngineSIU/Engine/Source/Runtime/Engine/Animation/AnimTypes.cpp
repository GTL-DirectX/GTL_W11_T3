
#include "AnimTypes.h"


float FAnimNotifyEvent::GetDuration() const
{
    return Duration;
}

float FAnimNotifyEvent::GetTriggerTime() const
{
    //return GetTime() + TriggerTimeOffset;
    return TriggerTime;
}

float FAnimNotifyEvent::GetEndTriggerTime() const
{
    //if (!NotifyStateClass && (EndTriggerTimeOffset != 0.f))
    //{
    //    UE_LOG(LogAnimNotify, Log, TEXT("Anim Notify %s is non state, but has an EndTriggerTimeOffset %f!"), *NotifyName.ToString(), EndTriggerTimeOffset);
    //}

    //return NotifyStateClass ? (GetTriggerTime() + GetDuration() + EndTriggerTimeOffset) : GetTriggerTime();

    return GetTriggerTime();
}
