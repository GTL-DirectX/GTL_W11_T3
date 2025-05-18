#pragma once

#include "Components/PrimitiveComponent.h"

struct FDynamicEmitterReplayDataBase;
class UParticleSystem;
struct FParticleEmitterInstance;
struct FDynamicEmitterDataBase;

class UParticleSystemComponent : public UPrimitiveComponent // UE는 UFXSystemComponent를 상속받음. 현재는 불필요할 것으로 생략하고 UPrimitiveComponent를 상속받음.
{
    DECLARE_CLASS(UParticleSystemComponent, UPrimitiveComponent)


public:
    UParticleSystemComponent() = default;
    virtual void TickComponent(float DeltaTime) override;

    void DeactivateSystem();
    void ComputeTickComponent_Concurrent(float DeltaTimeTick);

    static FDynamicEmitterDataBase* CreateDynamicDataFromReplay(FParticleEmitterInstance* EmitterInstance, const FDynamicEmitterReplayDataBase* EmitterReplayData, bool bSelected);

    virtual void InitParticles();


    TArray<struct FDynamicEmitterDataBase*> GetRenderData() { return EmitterRenderData; }

public:
    TArray<struct FParticleEmitterInstance*> EmitterInstances;

private:
    UParticleSystem* Template;

    TArray<struct FDynamicEmitterDataBase*> EmitterRenderData; // UE 클래스에서는 이 변수는 없고 CreateDynamicDataFromReplay() 함수가 있음

    uint8 bSuppressSpawning : 1 = true;


};

