#pragma once
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystemComponent.h"
#include "UnrealEd/EditorPanel.h"

class UParticleModule;
class UParticleEmitter;
#include "PropertyEditor/ImGuiInspector.h"

class UParticleSystem;

class ParticleSystemViewerPanel : public UEditorPanel
{
public:
    ParticleSystemViewerPanel() = default;
    ~ParticleSystemViewerPanel() = default;

    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    void RenderParticles();
    void RenderEmitters();
    void RenderDetails();
    void RenderCurveEditor();

    template <class T>
    void RenderProperties(T* Obj);

    void RenderModuleItem(UParticleEmitter* Emitter, UParticleModule* Module);
    
    // Test
    UParticleEmitter* CreateDefaultEmitter(int32 Index);
    
private:
    float Width = 800, Height = 600;

    UParticleSystem* CurrentParticleSystem = nullptr;
    UParticleSystemComponent* CurrentParticleSystemComponent = nullptr;
    
    UParticleEmitter* SelectedEmitter = nullptr;
    std::string SelectedModuleName;
    UParticleModule* SelectedModule = nullptr;
    int DefaultEmitterIndex = -1;
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
