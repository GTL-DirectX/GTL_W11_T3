#pragma once
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystemComponent.h"
#include "UnrealEd/EditorPanel.h"

#include "PropertyEditor/ImGuiInspector.h"

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

    template <class T>
    void RenderProperties(T* Obj);

private:
    float Width = 800, Height = 600;

    UParticleSystem* SelectedParticleSystem = nullptr;
    UParticleSystemComponent* SelectedParticleSystemComponent = nullptr;
    TArray<struct FDynamicEmitterDataBase*> EmitterRenderData;

    int SelectedEmitterIndex = -1;
};


template<typename T>
void ParticleSystemViewerPanel::RenderProperties(T* Obj)
{
    if (!Obj)
    {
        return;
    }
    UObject* UObjectPtr = Cast<UObject>(Obj);
    UClass* Cls = UObjectPtr->GetClass();

    // 등록된 모든 UField 를 순회하면서 DrawFieldEditor 호출
    Cls->ForEachField([&](UField* Field)
        {
            ImGuiInspector::DrawFieldEditor(Field, UObjectPtr);
            ImGui::Spacing();
        });
}
