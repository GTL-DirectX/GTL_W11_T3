#include "ParticleSystemViewerPanel.h"
#include "Engine/EditorEngine.h"
#include "Particles/ParticleSystem.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleModuleColor.h"
#include "Particles/ParticleModuleLifeTime.h"
#include "Particles/ParticleModuleRequired.h"
#include "Particles/ParticleModuleSize.h"
#include "Particles/ParticleModuleSpawn.h"
#include "Particles/ParticleModuleVelocity.h"
#include "Particles/ParticleSystemComponent.h"

void ParticleSystemViewerPanel::Render()
{
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
        return;

    if (!CurrentParticleSystemComponent)
    {
        CurrentParticleSystemComponent = Engine->GetSelectedActor()->GetComponentByClass<UParticleSystemComponent>();
        CurrentParticleSystem = CurrentParticleSystemComponent->GetParticleSystem();
    }
    
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
    ImGui::Begin("Emitters", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

    ImGui::SameLine();
    if (ImGui::Button("Add Emitter"))
    {
        DefaultEmitterIndex++;
        UParticleEmitter* NewEmitter = CreateDefaultEmitter(DefaultEmitterIndex);
        if (NewEmitter)
        {
            if (!CurrentParticleSystem)
            {
                CurrentParticleSystem = new UParticleSystem();
                CurrentParticleSystemComponent->SetParticleSystem(CurrentParticleSystem);
            }
            CurrentParticleSystem->Emitters.Add(NewEmitter);
        }
    }

    if (!CurrentParticleSystemComponent || !CurrentParticleSystem)
    {
        ImGui::End();
        return;
    }
    
    auto& Emitters = CurrentParticleSystem->Emitters;
    float EmitterWidth       = 220.0f;
    float totalContentWidth  = EmitterWidth * float(Emitters.Num());
    ImGui::SetNextWindowContentSize(ImVec2(totalContentWidth, 0));
    
    for (int i = 0; i < Emitters.Num(); ++i)
    {
        UParticleEmitter* Emitter = Emitters[i];
        ImGui::BeginChild(("Emitter" + std::to_string(i)).c_str(), ImVec2(EmitterWidth, 0), true);
        
        //── 1. Emitter Base Info 선택 영역 ───────────────────────────────
        float regionWidth = ImGui::GetContentRegionAvail().x;
        ImVec2 regionSize = ImVec2(regionWidth, 100);
        bool isEmitterSelected = Emitter == SelectedEmitter;
        std::string EmitterName = GetData(*Emitter->EmitterName.ToString());
        if (ImGui::Selectable(EmitterName.c_str(), isEmitterSelected, ImGuiSelectableFlags_None, regionSize))
        {
            SelectedEmitter = Emitter;
            SelectedModule = nullptr;
        }
            
        //── 2. Required 모듈 ─────────────────────────────────────────────
        UParticleModuleRequired* RequiredModule = Emitter->LODLevels[0]->RequiredModule;
        RenderModuleItem(Emitter, RequiredModule, "Required");

        //── 3. 나머지 모듈들 ──────────────────────────────────────────────
        auto& Modules = Emitter->LODLevels[0]->Modules;
        for (int m=0; m < Modules.Num(); ++m)
        {
            std::string ModuleName = "Module_" + std::to_string(m);
            RenderModuleItem(Emitter, Modules[m], ModuleName);
        }
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
    if (SelectedEmitter)
    {
        std::string EmitterName = GetData(*SelectedEmitter->EmitterName.ToString());
        ImGui::Text("Emitter : %s", EmitterName.c_str());
    }
    if (SelectedModule)
        ImGui::Text("Module  : %s", SelectedModuleName.c_str());
    ImGui::End();
}

void ParticleSystemViewerPanel::RenderCurveEditor()
{
    ImGui::SetNextWindowPos(ImVec2(Width * 0.3f, Height * 0.55f));
    ImGui::SetNextWindowSize(ImVec2(Width * 0.7f, Height * 0.45f));
    ImGui::Begin("Curve Editor", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    
    ImGui::End();
}

UParticleEmitter* ParticleSystemViewerPanel::CreateDefaultEmitter(int32 Index)
{
    // 1) Emitter 객체 생성
    UParticleEmitter* NewEmitter = FObjectFactory::ConstructObject<UParticleEmitter>(CurrentParticleSystem);
    std::string EmitterName = "DefaultEmitter_" + std::to_string(Index);
    NewEmitter->EmitterName = EmitterName.c_str();
    NewEmitter->ParticleSize = 20;

    // 2) LODLevel 0 생성 및 기본 설정
    UParticleLODLevel* LOD0 = FObjectFactory::ConstructObject<UParticleLODLevel>(NewEmitter);
    LOD0->LODLevel = 0;
    LOD0->bEnabled = true;

    // --- 3) Required 모듈 (필수) ---
    {
        UParticleModuleRequired* Required = FObjectFactory::ConstructObject<UParticleModuleRequired>(LOD0);
        Required->EmitterOrigin   = FVector::ZeroVector;
        Required->EmitterRotation = FRotator::ZeroRotator;
        LOD0->RequiredModule = Required;
    }

    // --- 4) Spawn 모듈 (생성 빈도) ---
    {
        UParticleModuleSpawn* Spawn = FObjectFactory::ConstructObject<UParticleModuleSpawn>(LOD0);
        LOD0->Modules.Add(Spawn);
    }

    // --- 5) Lifetime 모듈 (수명) ---
    {
        UParticleModuleLifeTime* Life = FObjectFactory::ConstructObject<UParticleModuleLifeTime>(LOD0);
        LOD0->Modules.Add(Life);
    }

    // --- 6) Initial Size 모듈 (크기) ---
    {
        UParticleModuleSize* Size = FObjectFactory::ConstructObject<UParticleModuleSize>(LOD0);
        LOD0->Modules.Add(Size);
    }

    // --- 7) Initial Velocity 모듈 (초기 속도) ---
    {
        UParticleModuleVelocity* Vel = FObjectFactory::ConstructObject<UParticleModuleVelocity>(LOD0);
        LOD0->Modules.Add(Vel);
    }

    // --- 8) Color Over Time 모듈 (색상 변화) ---
    {
        UParticleModuleColor* Color = FObjectFactory::ConstructObject<UParticleModuleColor>(LOD0);
        LOD0->Modules.Add(Color);
    }

    // 9) LODLevel을 Emitter에 추가
    NewEmitter->LODLevels.Add(LOD0);

    return NewEmitter;
}

void ParticleSystemViewerPanel::RenderModuleItem(UParticleEmitter* Emitter, UParticleModule* Module, const std::string& ModuleName)
{
    bool isSelected = (Module == SelectedModule);
    if (ImGui::Selectable(ModuleName.c_str(), isSelected))
    {
        SelectedEmitter    = Emitter;
        SelectedModuleName = ModuleName;
        SelectedModule     = Module;
    }
}
