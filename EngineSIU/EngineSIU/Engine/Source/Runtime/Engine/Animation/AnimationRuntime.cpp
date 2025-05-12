#include "AnimationRuntime.h"

#include "AnimSequenceBase.h"


/* 애니메이션의 현재 재생 시간을 MoveDelta 만큼 이동
 * 이동 후 루프 처리 또는 종료 처리 여부 판별
 */
ETypeAdvanceAnim FAnimationRuntime::AdvanceTime(const bool bAllowLooping, const float MoveDelta, float& InOutTime, const float EndTime)
{
    float NewTime = InOutTime + MoveDelta;

    /* 시간 이동 후 범위를 벗어난다면 */
    if (NewTime < 0.f || NewTime > EndTime)
    {
        if (bAllowLooping)
        {
            if (EndTime != 0.f)
            {
                NewTime = FMath::Fmod(NewTime, EndTime);
                if (NewTime < 0.f) // 나머지 연산 결과 음수 예외 처리
                {
                    NewTime += EndTime;
                }
            }
            else // 루프가 가능하지만, 애니메이션 길이가 0인 경우
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
