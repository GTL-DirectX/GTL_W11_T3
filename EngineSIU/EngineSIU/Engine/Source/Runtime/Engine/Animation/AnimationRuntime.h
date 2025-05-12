#pragma once

enum ETypeAdvanceAnim : int;

class FAnimationRuntime
{
public:
    static ETypeAdvanceAnim AdvanceTime(const bool bAllowLooping, const float MoveDelta, float& InOutTime, const float EndTime);
};

