#include "AnimNotifyQueue.h"

void FAnimNotifyQueue::AddAnimNotifies(const TArray<FAnimNotifyEvent>& InAnimNotifies)
{
    AnimNotifies.Append(InAnimNotifies);
}

void FAnimNotifyQueue::Reset()
{
    // 원본 코드에서 Comp받아오는 이유 : 먼 거리에서 Notify가 발생하면 작거나 보이지 않습니다.
    // 언리얼에선 NotifyFilterLOD로 최소 몇 이하에서만 실행할지 결정할 수 있습니다.
    AnimNotifies.Empty();
}
