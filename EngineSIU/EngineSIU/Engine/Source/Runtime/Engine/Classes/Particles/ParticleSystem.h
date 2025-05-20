#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class UParticleEmitter;

class UParticleSystem : public UObject // UE는 UFXSystemAsset을 상속. 불필요해 보이므로 생략.
{
    DECLARE_CLASS(UParticleSystem, UObject)

public:
    UParticleSystem() = default;
    void BuildEmitters();

    //virtual void PostLoad();

    // 에디터에서 생성한 UParticleEmitter들
    TArray<UParticleEmitter*> Emitters;


};
