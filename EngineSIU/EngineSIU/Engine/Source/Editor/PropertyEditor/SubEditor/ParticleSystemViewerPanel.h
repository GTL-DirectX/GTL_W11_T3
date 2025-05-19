#pragma once

#include "Define.h"
#include "Container/Array.h"
#include "ImGui/imgui.h"
#include "UnrealEd/EditorPanel.h"

class UParticleSystem;
class UParticleSystemComponent;

class ParticleSystemViewerPanel : public UEditorPanel
{
public:
    ParticleSystemViewerPanel() = default;
    ~ParticleSystemViewerPanel() = default;

    virtual void Init() override;
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

    FRect GetViewportSize() const { return ViewportSize; }

private:
    void RenderMainViewport();
    void RenderEmitters();
    void RenderDetails();
    void RenderCurveEditor();

private:
    float Width = 800, Height = 600;

    FRect ViewportSize;

    UParticleSystem* SelectedParticleSystem = nullptr;
    UParticleSystemComponent* SelectedParticleSystemComponent = nullptr;
    TArray<struct FDynamicEmitterDataBase*> EmitterRenderData;

    int SelectedEmitterIndex = -1;
};

