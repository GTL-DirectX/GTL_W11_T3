#pragma once
#include "GameFramework/Actor.h"
#include "UnrealEd/EditorPanel.h"

class ParticleSystemEditorPanel : public UEditorPanel
{
public:
    ParticleSystemEditorPanel() = default;
    ~ParticleSystemEditorPanel() = default;
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    float Width = 800, Height = 600;
};

