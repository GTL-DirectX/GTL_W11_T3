#pragma once
#pragma once
#include "AnimInstance.h"
#include "Animation/AnimSequenceBase.h"


// 하나의 애니메이션 클립에 대한 시간 진행 상태와 재생 속도 정보를 캡슐화.
struct FBlendState
{
    UAnimSequenceBase* Sequence = nullptr;      // 애니메이션 시퀀스 포인터
    float CurrentTime = 0.0f;       // 애니메이션 내에서의 재생 시간 (초 단위)
    float RateScale = 1.0f;         // 재생 속도 배율

    // 외부 Tick 시간에 따라 CurrentTime 자동 증가.
    void Advance(float DeltaSeconds) {
        CurrentTime += DeltaSeconds * RateScale;
    }

    // 유효한 애니메이션 상태 확인.
    bool IsValid() const { return Sequence != nullptr; }
};

//class UCurveFloat;

/*
*    두 개의 애니메이션을 블랜딩하여 중간 결과 포즈 생성하는 애니메이션 인스턴스
*/
class UAnimTwoNodeBlendInstance : public UAnimInstance
{
    DECLARE_CLASS(UAnimTwoNodeBlendInstance, UAnimInstance)
public:
    UAnimTwoNodeBlendInstance() = default;
    
    // bResetTimeAndState가 true면 두 애니메이션 시간 0으로 초기화.
    // InBlendTime : 두 시퀀스 사이의 블렌딩 시간(초 단위).
    // 이미 같은 애니메이션이면 아무것도 하지 않도록.
    void SetAnimationAsset(UAnimSequenceBase* NewAnimToPlay, bool bResetTimeAndState, float InBlendTime = 0.2f);        // 블렌딩 대상 애니메이션(To) 설정

    // 매 프레임마다 호출되어 BlendAlpha 업데이트 후, 두 애니메이션의 시간을 전진.
    virtual void UpdateAnimation(float DeltaSeconds) override;

    // From과 To의 PoseA, PoseB를 계산 후 보간하여 최종 포즈 생성(CurrentPose)
    virtual const TArray<FTransform>& EvaluateAnimation() override;

    UAnimSequenceBase* GetFromSequence() const;
    UAnimSequenceBase* GetToSequence() const;

    void StopBlend(bool bResetPose = true);

    // TODO 전이 완료 시 델리게이트 호출
    //DECLARE_DELEGATE(FOnBlendComplete);
    // FOnBlendComplete OnBlendComplete;

    //void SetBlendCurve(UCurveFloat* InCurve);

protected:
    FBlendState From;               // 블렌딩 이전 상태(기준)와 블렌딩 대상 상태
    FBlendState To;

    float BlendAlpha = 0.0f;        // 0이면 SequenceA, 1이면 SequenceB
    float  BlendTime = 0.2f;        // 블렌딩에 걸리는 시간
    float ElaspedBlendTime = 0.0f;
    bool bIsBlending = false;
    
    // TODO : 블렌드 커브로 자연스럽게 변경
    //UCurveFloat* BlendCurve = nullptr;

    TArray<FTransform> PoseA;
    TArray<FTransform> PoseB;
};
