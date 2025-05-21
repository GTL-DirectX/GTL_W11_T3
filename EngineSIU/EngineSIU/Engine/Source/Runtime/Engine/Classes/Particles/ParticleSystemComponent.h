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
    virtual void PostInitProperties() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual void FinalizeTickComponent();

    void InitializeSystem();
    void DeactivateSystem();
    void ComputeTickComponent_Concurrent(float DeltaTimeTick);

    static FDynamicEmitterDataBase* CreateDynamicDataFromReplay(FParticleEmitterInstance* EmitterInstance, const FDynamicEmitterReplayDataBase* EmitterReplayData, bool bSelected);

    virtual void InitParticles();
    void ResetParticles();

    void SetTemplate(class UParticleSystem* NewTemplate);
    TArray<struct FDynamicEmitterDataBase*> GetRenderData() { return EmitterRenderData; }
    UParticleSystem* GetParticleSystem() const { return Template; }
    void SetParticleSystem(UParticleSystem* system) { Template = system; }

public:
    TArray<struct FParticleEmitterInstance*> EmitterInstances;
    UParticleSystem* Template = nullptr;

    TArray<struct FDynamicEmitterDataBase*> EmitterRenderData; // UE 클래스에서는 이 변수는 없고 CreateDynamicDataFromReplay() 함수가 있음

    /* [일시적인 억제 플래그] true이면 현재 프레임에서 파티클을 생성하지 않음*/
    uint8 bSuppressSpawning : 1;

    FVector InitialLocationHardcoded;
    FVector InitialVelocityHardcoded;

};

