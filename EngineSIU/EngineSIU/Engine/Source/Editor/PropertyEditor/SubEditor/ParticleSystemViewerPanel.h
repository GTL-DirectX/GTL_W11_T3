#pragma once

#include "Define.h"
#include "Container/Array.h"
#include "ImGui/imgui.h"
#include "UnrealEd/EditorPanel.h"
#include "UObject/Class.h"

class UParticleModule;
class UParticleEmitter;
class UParticleLODLevel;
#include "PropertyEditor/ImGuiInspector.h"

class UParticleSystem;
class UParticleSystemComponent;

class ParticleSystemViewerPanel : public UEditorPanel
{
public:
    ParticleSystemViewerPanel() = default;
    ~ParticleSystemViewerPanel() = default;

    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

    FRect GetViewportSize() const { return ViewportSize; }

private:
    void RenderMainViewport();
    void RenderEmitters();
    void RenderDetails();
    void RenderCurveEditor();

    template <class T>
    void RenderProperties(T* Obj);

    void RenderModuleItem(UParticleEmitter* Emitter, UParticleModule* Module);

    void RenderAddModulePopup(UParticleEmitter* module);
    
private:
    float Width = 800, Height = 600;

    FRect ViewportSize;

    UParticleSystem* SelectedParticleSystem = nullptr;
    UParticleSystemComponent* SelectedParticleSystemComponent = nullptr;
    TArray<struct FDynamicEmitterDataBase*> EmitterRenderData;

    int SelectedEmitterIndex = -1;
    UParticleSystem* CurrentParticleSystem = nullptr;
    UParticleSystemComponent* CurrentParticleSystemComponent = nullptr;
    
    UParticleEmitter* SelectedEmitter = nullptr;
    std::string SelectedModuleName;
    UParticleModule* SelectedModule = nullptr;
    int DefaultEmitterIndex = -1;

    bool bOpenAddModulePopup = false;
    int  PendingModuleIndex = 0;   // 사용자가 콤보에서 고른 인덱스
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
