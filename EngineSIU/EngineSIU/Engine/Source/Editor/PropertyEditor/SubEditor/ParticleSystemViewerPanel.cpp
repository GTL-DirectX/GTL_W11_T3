#include "ParticleSystemViewerPanel.h"
#include "Engine/EditorEngine.h"
#include "Particles/ParticleSystem.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleSystemComponent.h"

void ParticleSystemViewerPanel::Init()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
        return;

    SelectedParticleSystemComponent = Engine->GetSelectedActor()->GetComponentByClass<UParticleSystemComponent>();
    SelectedParticleSystem = SelectedParticleSystemComponent->GetParticleSystem();
    EmitterRenderData = SelectedParticleSystemComponent->GetRenderData();
    SelectedEmitterIndex = 0;
}

void ParticleSystemViewerPanel::Render()
{
    // Viewport
    RenderParticles();
    
    // Emitters
    RenderEmitters();

    // Details
    RenderDetails();

    // Curve Editor
    RenderCurveEditor();
}

void ParticleSystemViewerPanel::OnResize(HWND hWnd)
{
    RECT rect;
    if (GetClientRect(hWnd, &rect))
    {
        Width = static_cast<float>(rect.right - rect.left);
        Height = static_cast<float>(rect.bottom - rect.top);
    }
}

void ParticleSystemViewerPanel::RenderParticles()
{
    ImGui::SetNextWindowPos(ImVec2(0.f, 0.f));
    ImGui::SetNextWindowSize(ImVec2(Width * 0.3f, Height * 0.55f));
    ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    ImGui::End();
}

void ParticleSystemViewerPanel::RenderEmitters()
{
    ImGui::SetNextWindowPos(ImVec2(Width * 0.3f, 0.f));
    ImGui::SetNextWindowSize(ImVec2(Width * 0.7f, Height * 0.55f));

    // auto Emitters = SelectedParticleSystem->Emitters;
    float EmitterWidth = 220.0f;
    // float totalContentWidth = EmitterWidth * Emitters.Num();
    ImGui::SetNextWindowContentSize(ImVec2(220*8/*totalContentWidth*/, 0.0f));
    ImGui::Begin("Emitters", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    for (int i = 0; i < 8/*Emitters.Num()*/; ++i)
    {
        
        ImGui::BeginChild(("Emitter" + std::to_string(i)).c_str(), ImVec2(EmitterWidth, 0), true);
        
        // 1. Emitter Base Info
        //auto* Emitter = Emitters[i];

        //ImGui::Text("%s", Emitter->EmitterName.ToString());
        // bool enabled = Emitter->bEnabled;
        // ImGui::Checkbox("##Enabled", &enabled);
        //int particleCount = Emitter->

        // 2. Emitter Module List
        //auto Modules = Emitter->LODLevels[0]->Modules;
        // for (UParticleModule* Module : Modules)
        // {
        //      
        // }
        ImGui::EndChild();
        ImGui::SameLine();
    }
    
    ImGui::End();
}

void ParticleSystemViewerPanel::RenderDetails()
{
    ImGui::SetNextWindowPos(ImVec2(0.f, Height * 0.55f));
    ImGui::SetNextWindowSize(ImVec2(Width * 0.3f, Height * 0.45f));
    ImGui::Begin("Details", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    ImGui::End();
}

void ParticleSystemViewerPanel::RenderCurveEditor()
{
    ImGui::SetNextWindowPos(ImVec2(Width * 0.3f, Height * 0.55f));
    ImGui::SetNextWindowSize(ImVec2(Width * 0.7f, Height * 0.45f));
    ImGui::Begin("Curve Editor", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    ImGui::End();
}
