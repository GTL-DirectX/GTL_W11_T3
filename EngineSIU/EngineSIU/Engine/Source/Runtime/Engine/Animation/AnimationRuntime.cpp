#include "AnimationRuntime.h"

#include "AnimSequenceBase.h"

ETypeAdvanceAnim FAnimationRuntime::AdvanceTime(const bool bAllowLooping, const float MoveDelta, float& InOutTime, const float EndTime)
{
    float NewTime = InOutTime + MoveDelta;

    if (NewTime < 0.f || NewTime > EndTime)
    {
        if (bAllowLooping)
        {
            if (EndTime != 0.f)
            {
                NewTime = FMath::Fmod(NewTime, EndTime);
                if (NewTime < 0.f)
                {
                    NewTime += EndTime;
                }
            }
            else
            {
                NewTime = 0.f;
            }

            InOutTime = NewTime;
            return ETAA_Looped;
        }
        else
        {
            InOutTime = FMath::Clamp(NewTime, 0.f, EndTime);
            return ETAA_Finished;
        }
    }

    InOutTime = NewTime;
    return ETAA_Default;
}
