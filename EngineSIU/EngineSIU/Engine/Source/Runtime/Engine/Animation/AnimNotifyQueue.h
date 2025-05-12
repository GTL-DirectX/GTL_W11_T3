#pragma once
#include "AnimTypes.h"
#include "Container/Array.h"


struct FAnimNotifyQueue
{
    TArray<FAnimNotifyEvent> AnimNotifies;

    void AddAnimNotifies(const TArray<FAnimNotifyEvent>& InAnimNotifies);

    void Reset();
};
