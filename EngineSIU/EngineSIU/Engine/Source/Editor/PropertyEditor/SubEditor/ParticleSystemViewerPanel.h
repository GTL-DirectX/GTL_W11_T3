#pragma once
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystemComponent.h"
#include "UnrealEd/EditorPanel.h"

class UParticleSystem;

class ParticleSystemViewerPanel : public UEditorPanel
{
public:
    ParticleSystemViewerPanel() = default;
    ~ParticleSystemViewerPanel() = default;

    virtual void Init() override;
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    void RenderParticles();
    void RenderEmitters();
    void RenderDetails();
    void RenderCurveEditor();

private:
    float Width = 800, Height = 600;

    UParticleSystem* SelectedParticleSystem = nullptr;
    UParticleSystemComponent* SelectedParticleSystemComponent = nullptr;
    TArray<struct FDynamicEmitterDataBase*> EmitterRenderData;

    int SelectedEmitterIndex = -1;
};

