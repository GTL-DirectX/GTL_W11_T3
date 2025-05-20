#include "ParticleSystemViewerPanel.h"

#include "UnrealClient.h"
#include "Engine/EditorEngine.h"
#include "GameFramework/Actor.h"
#include "ImGui/imgui_internal.h"
#include "LevelEditor/SLevelEditor.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleEmitter.h"
#include "Particles/ParticleLODLevel.h"
#include "Particles/ParticleModuleColor.h"
#include "Particles/ParticleModuleLifeTime.h"
#include "Particles/ParticleModuleRequired.h"
#include "Particles/ParticleModuleSize.h"
#include "Particles/ParticleModuleSpawn.h"
#include "Particles/ParticleModuleVelocity.h"
#include "Particles/ParticleSystemComponent.h"
#include "UnrealEd/EditorViewportClient.h"
#include "Engine/AssetManager.h"

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
    RenderMainViewport();
    
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

void ParticleSystemViewerPanel::RenderMainViewport()
{
    ImGui::SetNextWindowPos(ImVec2(Width * 0.3f, 0.f));

    float ViewportWidth = Width * 0.3f;
    float ViewportHeight = Height * 0.55f;
    UE_LOG(ELogLevel::Display, TEXT("Client Size: %f %f"), Width, Height);
    UE_LOG(ELogLevel::Display, TEXT("ViewportPanelSize: %f %f"), ViewportWidth, ViewportHeight);

    ViewportSize = FRect{
        0,
        0,
        ViewportWidth,
        ViewportHeight
    };
}

void ParticleSystemViewerPanel::RenderEmitters()
{
    ImGui::SetNextWindowPos(ImVec2(Width * 0.3f, 0.f));
    ImGui::SetNextWindowSize(ImVec2(Width * 0.7f, Height * 0.55f));
    ImGui::Begin(
        "Emitters",
        nullptr,
        ImGuiWindowFlags_HorizontalScrollbar |
        ImGuiWindowFlags_NoMove        |
        ImGuiWindowFlags_NoResize      |
        ImGuiWindowFlags_NoCollapse
    );
    ImGui::SameLine();
    
    if (ImGui::Button("Simulate"))
    {
        // simulation logic
    }

    ////가로로 나열된 형태의 UI, Popup 형태 시도
    //ImGui::SameLine();
    //static char SaveSystemName[128] = "MyParticleSystem";
    //
    //ImGui::PushItemWidth(150);
    //ImGui::InputText("##SaveSysName", SaveSystemName, IM_ARRAYSIZE(SaveSystemName));
    //ImGui::PopItemWidth();

    ImGui::SameLine();

    if (ImGui::Button("Save System"))
    {
        ImGui::OpenPopup("Save System");
    }
    // ─── 모달 팝업 처리 ───
    // BeginPopupModal은 매 프레임 호출
    if (ImGui::BeginPopupModal(
        "Save System",   // ← OpenPopup의 ID와 정확히 일치해야 함
        nullptr,         // nullptr 주면 X 버튼은 생기지 않지만, Cancel 버튼으로만 닫음
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        static char SaveSystemName[128] = "";

        ImGui::Text("Enter name for this particle system:");
        ImGui::PushItemWidth(200);
        ImGui::InputText("##SaveSysName", SaveSystemName, IM_ARRAYSIZE(SaveSystemName));
        ImGui::PopItemWidth();

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(100, 0)))
        {
            UE_LOG(ELogLevel::Warning, TEXT("CurrentParticleSystem is %s and name is %s"), CurrentParticleSystem != nullptr ? TEXT("Valid") : TEXT("Null"), SaveSystemName);

            // SaveSystemName 을 이용해 실제 저장 처리
            FString Key(SaveSystemName); 

            //// 1) Get() 으로 매니저 참조 가져오기
            //UAssetManager& AssetMgr = UAssetManager::Get();
            ////UAssetManager::Get().AddSavedParticle(Key, CurrentParticleSystem);

            //// 2) 바로 Map 참조 선언과 초기화
            //auto& Map = AssetMgr.SavedParticleSystemMap;

            //Map.Emplace(Key, CurrentParticleSystem);

            UParticleSystem* SystemPtr = CurrentParticleSystem;

            // Get() 이 반환하는 UAssetMaanger& 에 바로 호출
            UAssetManager::Get().AddSavedParticleSystem(Key, SystemPtr);
   
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(100, 0)))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }

    //ImGui::SameLine(); 한 줄 아래에 Add, Rename, Delete 버튼 배치
    ImGui::Text("Emitter");
    ImGui::SameLine();
    if (ImGui::Button("Add"))
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

    // 선택된 emitter 이름변경하기 제거하기
    if (SelectedEmitter)
    {
        ImGui::SameLine();
        if (ImGui::Button("Rename"))
        {
            ImGui::OpenPopup("Rename");
        }
        
        ImGui::SameLine();
        if (ImGui::Button("Delete"))
        {
            // 배열에서 제거
            auto& Emitters = CurrentParticleSystem->Emitters;
            Emitters.RemoveSingle(SelectedEmitter);
            // 선택 해제
            SelectedEmitter = nullptr;
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
        bool isEmitterSelected = (Emitter == SelectedEmitter);

        // 하이라이트 처리 : BeginChild 의 border 인자로 isSelected 전달
        if (isEmitterSelected)
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.4, 0.8, 0.25f));

        ImGui::BeginChild(
            ("Emitter" + std::to_string(i)).c_str(),
            ImVec2(EmitterWidth, 0), 
            isEmitterSelected // 여기에 true 이면 테두리가 그려짐. 
        );
        
        //── 1. Emitter Base Info 선택 영역 ───────────────────────────────
        float regionWidth = ImGui::GetContentRegionAvail().x;
        ImVec2 regionSize = ImVec2(regionWidth, 100);
        //bool isEmitterSelected = (Emitter == SelectedEmitter);
        std::string EmitterName = GetData(*Emitter->EmitterName.ToString());

        if (ImGui::Selectable(EmitterName.c_str(), isEmitterSelected, ImGuiSelectableFlags_None, regionSize))
        {
            SelectedEmitter = Emitter;
            SelectedModule = nullptr;
        }
            
        //── 2. Required 모듈 ─────────────────────────────────────────────
        // UParticleModuleRequired* RequiredModule = Emitter->LODLevels[0]->RequiredModule;
        // RenderModuleItem(Emitter, RequiredModule);

        //── 3. 나머지 모듈들 ──────────────────────────────────────────────
        auto& Modules = Emitter->LODLevels[0]->Modules;
        for (int m=0; m < Modules.Num(); ++m)
        {
            RenderModuleItem(Emitter, Modules[m]);
        }
        ImGui::EndChild();

        if (isEmitterSelected)
            ImGui::PopStyleColor();

        ImGui::SameLine();
    }
    ImGui::End();
}

void ParticleSystemViewerPanel::RenderDetails()
{
    ImGui::SetNextWindowPos(ImVec2(0.f, Height * 0.55f));
    ImGui::SetNextWindowSize(ImVec2(Width * 0.3f, Height * 0.45f));
    ImGui::Begin("Details", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    if (SelectedEmitter)
    {
        std::string EmitterName = GetData(*SelectedEmitter->EmitterName.ToString());
        ImGui::Text("Emitter : %s", EmitterName.c_str());
    }
    if (SelectedModule)
    {
        ImGui::Text("Module  : %s", *SelectedModule->GetName());
        RenderProperties(SelectedModule);
    }
    ImGui::End();
}

void ParticleSystemViewerPanel::RenderCurveEditor()
{
    ImGui::SetNextWindowPos(ImVec2(Width * 0.3f, Height * 0.55f));
    ImGui::SetNextWindowSize(ImVec2(Width * 0.7f, Height * 0.45f));
    ImGui::Begin("Curve Editor", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
    
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

void ParticleSystemViewerPanel::RenderModuleItem(UParticleEmitter* Emitter, UParticleModule* Module)
{
    bool isSelected = (Module == SelectedModule);
    std::string RawName = *Module->GetName().ToString();
    const std::string Prefix = "UParticleModule";
    if (RawName.rfind(Prefix, 0) == 0)
        RawName = RawName.substr(Prefix.size());
    auto pos = RawName.find('_');
    if (pos != std::string::npos)
        RawName = RawName.substr(0, pos);
    
    if (ImGui::Selectable(RawName.c_str(), isSelected))
    {
        SelectedEmitter    = Emitter;
        SelectedModuleName = RawName;
        SelectedModule     = Module;
    }
}
