#include "ParticleSystemViewerPanel.h"

#include "UnrealClient.h"
#include "Engine/EditorEngine.h"
#include "Particles/ParticleActor.h"
#include "ImGui/imgui_internal.h"
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
        UWorld* World = Engine->GetPreviewWorld(GEngineLoop.ParticleSystemViewerAppWnd);
        if (!World)
            return;

        for (auto Actor : World->GetActiveLevel()->Actors)
        {
            if (Actor && Actor->IsA<AParticleActor>())
            {
                CurrentParticleSystemComponent = Actor->GetComponentByClass<UParticleSystemComponent>();
                CurrentParticleSystem = CurrentParticleSystemComponent->GetParticleSystem();
            }
        }
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

    const ImVec4 SelectedColor = ImVec4(0.2f, 0.4f, 0.8f, 0.4f); // 선택됨
    const ImVec4 HoveredColor = ImVec4(0.2f, 0.4f, 0.8f, 0.6f); // 마우스 올렸을 때
    const ImVec4 InactiveColor = ImVec4(0.2f, 0.4f, 0.8f, 0.2f); // 기본

    ImGui::PushStyleColor(ImGuiCol_Header, InactiveColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, HoveredColor);
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, SelectedColor);
    
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

    // 1) 팝업 위치를 기억할 static 변수
    static ImVec2 s_AddPopupPos = ImVec2(0, 0);
    //ImGui::SameLine(); 한 줄 아래에 Add, Rename, Delete 버튼 배치

    ImGui::Text("Emitter");
    ImGui::SameLine();
    if (ImGui::Button("Add"))
    {
        // 2-1) 버튼 우측 하단 좌표를 즉시 저장
        s_AddPopupPos = ImGui::GetItemRectMax();    // ← Add 버튼 위치만 기억
        // 2-2) 팝업 열기 요청
        ImGui::OpenPopup("Add Emitter Popup");
    }

    // 기존 Rename/Delete 버튼
    if (SelectedEmitter)
    {
        ImGui::SameLine();
        if (ImGui::Button("Rename"))
            ImGui::OpenPopup("Rename");

        ImGui::SameLine();
        if (ImGui::Button("Delete"))
        {
            auto& Emitters = CurrentParticleSystem->Emitters;
            Emitters.RemoveSingle(SelectedEmitter);
            SelectedEmitter = nullptr;
        }
    }

    ImGui::SetNextWindowPos(s_AddPopupPos, ImGuiCond_Appearing);

    // 4) 팝업 그리기
    if (ImGui::BeginPopup("Add Emitter Popup", ImGuiWindowFlags_AlwaysAutoResize))
    {
        bool bIsSpriteEmitter = false;
        if (ImGui::Selectable("New Particle Sprite Emitter"))
        {
            bIsSpriteEmitter = true;

            DefaultEmitterIndex++;
            UParticleEmitter* NewEmitter = CreateDefaultEmitter(DefaultEmitterIndex, bIsSpriteEmitter);
            if (NewEmitter)
            {
                if (!CurrentParticleSystem)
                {
                    CurrentParticleSystem = new UParticleSystem();
                    CurrentParticleSystemComponent->SetParticleSystem(CurrentParticleSystem);
                }
                CurrentParticleSystem->Emitters.Add(NewEmitter);
            }
            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Selectable("New Particle Mesh Emitter"))
        {
            bIsSpriteEmitter = false;
            
            DefaultEmitterIndex++;
            UParticleEmitter* NewEmitter = CreateDefaultEmitter(DefaultEmitterIndex, bIsSpriteEmitter);
            if (NewEmitter)
            {
                if (!CurrentParticleSystem)
                {
                    CurrentParticleSystem = new UParticleSystem();
                    CurrentParticleSystemComponent->SetParticleSystem(CurrentParticleSystem);
                }
                CurrentParticleSystem->Emitters.Add(NewEmitter);
            }
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
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
        std::string EmitterName = *Emitter->EmitterName.ToString();
        std::string UniqueChildID = "EmitterCard##" + std::to_string(i);

        if (isEmitterSelected)
            ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.4, 0.8, 0.1f));

        // Emitter 전체 박스
        ImGui::BeginChild(UniqueChildID.c_str(), ImVec2(EmitterWidth, 0), true);

        // 🟡 전체 박스 클릭 감지 추가
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            SelectedEmitter = Emitter;
            SelectedModule = nullptr;
        }

        // 🔹 Emitter 이름 라벨 (시각용)
        ImVec2 regionSize = ImVec2(ImGui::GetContentRegionAvail().x, 30.0f);
        ImGui::Selectable(EmitterName.c_str(), isEmitterSelected, ImGuiSelectableFlags_Disabled, regionSize);

        ImGui::Separator();

        // 2. 모듈 리스트 영역
        {
            ImGui::BeginChild(("Modules##" + std::to_string(i)).c_str(), ImVec2(0, 300), false);

            auto & Modules = Emitter->LODLevels[0]->Modules;
            for (int m = 0; m < Modules.Num(); ++m)
            {
                RenderModuleItem(Emitter, Modules[m]);
            }
       
            ImGui::EndChild();
        }

        // 3. Add Module 버튼
        if (ImGui::Button(("Add Module##" + std::to_string(i)).c_str()))
        {
            PendingModuleIndex = 0;
            SelectedEmitter = Emitter; // 중요: 팝업에 넘길 Emitter 설정
            bOpenAddModulePopup = true;
            ImGui::OpenPopup(("Add Module##Popup" + std::to_string(i)).c_str());
        }

        // 팝업은 같은 프레임에 BeginPopupModal() 호출돼야 동작함
        if (ImGui::BeginPopupModal(("Add Module##Popup" + std::to_string(i)).c_str(), &bOpenAddModulePopup, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("Select Module Type:");

            TArray<UClass*> ModuleClasses;
            GetChildOfClass(UParticleModule::StaticClass(), ModuleClasses);

            std::string ModuleName = GetData(*ModuleClasses[PendingModuleIndex]->GetName());
            if (ImGui::BeginCombo("##ModuleType", ModuleName.c_str()))
            {
                for (int j = 0; j < ModuleClasses.Num(); ++j)
                {
                    UClass* Class = ModuleClasses[j];
                    if (ImGui::Selectable(GetData(Class->GetName()), PendingModuleIndex == j))
                    {
                        PendingModuleIndex = j;
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }

            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(100, 0)))
            {
                if (SelectedEmitter)
                {
                    SelectedEmitter->LODLevels[0]->AddModule(ModuleClasses[PendingModuleIndex]);
                }
                bOpenAddModulePopup = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(100, 0)))
            {
                bOpenAddModulePopup = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::EndChild();

        if (isEmitterSelected)
            ImGui::PopStyleColor();

        ImGui::SameLine();
    }

    ImGui::PopStyleColor(3);

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
        // (1) 원래 LOD0->RequiredModule이 가리키던 '기본 Required'를 임시로 저장
        UParticleModuleRequired* OldRequired = LOD0->RequiredModule;

        // (2) 새 Required 생성·설정
        UParticleModuleRequired* Required = FObjectFactory::ConstructObject<UParticleModuleRequired>(LOD0);
        Required->EmitterOrigin   = FVector::ZeroVector;
        Required->EmitterRotation = FRotator::ZeroRotator;
        LOD0->RequiredModule = Required;
        // (3) 기존 Modules 복사
        TArray<UParticleModule*> Temp = LOD0->Modules;

        // (4) 복사해 온 배열에서 '기본 Required' 하나만 제거
        Temp.RemoveSingle(OldRequired);

        // (5) 원본 비우고, 새 Required를 맨 앞에 추가
        LOD0->Modules.Empty();
        LOD0->Modules.Add(Required);

        // (6) 나머지(필터링된) 모듈들 붙이기
        LOD0->Modules.Append(Temp);
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

UParticleEmitter* ParticleSystemViewerPanel::CreateDefaultEmitter(int32 Index, bool bIsSpriteEmitter)
{
    // 1) Emitter 객체 생성
    UParticleEmitter* NewEmitter = FObjectFactory::ConstructObject<UParticleEmitter>(CurrentParticleSystem);
    if (bIsSpriteEmitter)
    {
        std::string EmitterName = "DefaultSpriteEmitter_" + std::to_string(Index);
        NewEmitter->EmitterName = EmitterName.c_str();
        NewEmitter->ParticleSize = 20;
    }
    else
    {
        std::string EmitterName = "DefaultMeshEmitter_" + std::to_string(Index);
        NewEmitter->EmitterName = EmitterName.c_str();
        NewEmitter->ParticleSize = 20;
    }

    // 2) LODLevel 0 생성 및 기본 설정
    UParticleLODLevel* LOD0 = FObjectFactory::ConstructObject<UParticleLODLevel>(NewEmitter);
    LOD0->LODLevel = 0;
    LOD0->bEnabled = true;

    // --- 3) Required 모듈 (필수) ---
    {
        // (1) 원래 LOD0->RequiredModule이 가리키던 '기본 Required'를 임시로 저장
        UParticleModuleRequired* OldRequired = LOD0->RequiredModule;

        // (2) 새 Required 생성·설정
        UParticleModuleRequired* Required = FObjectFactory::ConstructObject<UParticleModuleRequired>(LOD0);
        Required->EmitterOrigin = FVector::ZeroVector;
        Required->EmitterRotation = FRotator::ZeroRotator;
        LOD0->RequiredModule = Required;
        // (3) 기존 Modules 복사
        TArray<UParticleModule*> Temp = LOD0->Modules;

        // (4) 복사해 온 배열에서 '기본 Required' 하나만 제거
        Temp.RemoveSingle(OldRequired);

        // (5) 원본 비우고, 새 Required를 맨 앞에 추가
        LOD0->Modules.Empty();
        LOD0->Modules.Add(Required);

        // (6) 나머지(필터링된) 모듈들 붙이기
        LOD0->Modules.Append(Temp);
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

UParticleEmitter* ParticleSystemViewerPanel::CreateDefaultSpriteEmitter(int32 Index)
{
    // 1) Emitter 객체 생성
    UParticleEmitter* NewEmitter = FObjectFactory::ConstructObject<UParticleEmitter>(CurrentParticleSystem);
    std::string EmitterName = "DefaultSpriteEmitter_" + std::to_string(Index);
    NewEmitter->EmitterName = EmitterName.c_str();
    NewEmitter->ParticleSize = 20;

    // 2) LODLevel 0 생성 및 기본 설정
    UParticleLODLevel* LOD0 = FObjectFactory::ConstructObject<UParticleLODLevel>(NewEmitter);
    LOD0->LODLevel = 0;
    LOD0->bEnabled = true;

    // --- 3) Required 모듈 (필수) ---
    {
        // (1) 원래 LOD0->RequiredModule이 가리키던 '기본 Required'를 임시로 저장
        UParticleModuleRequired* OldRequired = LOD0->RequiredModule;

        // (2) 새 Required 생성·설정
        UParticleModuleRequired* Required = FObjectFactory::ConstructObject<UParticleModuleRequired>(LOD0);
        Required->EmitterOrigin = FVector::ZeroVector;
        Required->EmitterRotation = FRotator::ZeroRotator;
        LOD0->RequiredModule = Required;
        // (3) 기존 Modules 복사
        TArray<UParticleModule*> Temp = LOD0->Modules;

        // (4) 복사해 온 배열에서 '기본 Required' 하나만 제거
        Temp.RemoveSingle(OldRequired);

        // (5) 원본 비우고, 새 Required를 맨 앞에 추가
        LOD0->Modules.Empty();
        LOD0->Modules.Add(Required);

        // (6) 나머지(필터링된) 모듈들 붙이기
        LOD0->Modules.Append(Temp);
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

UParticleEmitter* ParticleSystemViewerPanel::CreateDefaultMeshEmitter(int32 Index)
{
    // 1) Emitter 객체 생성
    UParticleEmitter* NewEmitter = FObjectFactory::ConstructObject<UParticleEmitter>(CurrentParticleSystem);
    std::string EmitterName = "DefaultMeshEmitter_" + std::to_string(Index);
    NewEmitter->EmitterName = EmitterName.c_str();
    NewEmitter->ParticleSize = 20;

    // 2) LODLevel 0 생성 및 기본 설정
    UParticleLODLevel* LOD0 = FObjectFactory::ConstructObject<UParticleLODLevel>(NewEmitter);
    LOD0->LODLevel = 0;
    LOD0->bEnabled = true;

    // --- 3) Required 모듈 (필수) ---
    {
        // (1) 원래 LOD0->RequiredModule이 가리키던 '기본 Required'를 임시로 저장
        UParticleModuleRequired* OldRequired = LOD0->RequiredModule;

        // (2) 새 Required 생성·설정
        UParticleModuleRequired* Required = FObjectFactory::ConstructObject<UParticleModuleRequired>(LOD0);
        Required->EmitterOrigin = FVector::ZeroVector;
        Required->EmitterRotation = FRotator::ZeroRotator;
        LOD0->RequiredModule = Required;
        // (3) 기존 Modules 복사
        TArray<UParticleModule*> Temp = LOD0->Modules;

        // (4) 복사해 온 배열에서 '기본 Required' 하나만 제거
        Temp.RemoveSingle(OldRequired);

        // (5) 원본 비우고, 새 Required를 맨 앞에 추가
        LOD0->Modules.Empty();
        LOD0->Modules.Add(Required);

        // (6) 나머지(필터링된) 모듈들 붙이기
        LOD0->Modules.Append(Temp);
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
    std::string DisplayName = "##" + std::to_string(reinterpret_cast<uintptr_t>(Module));

    const std::string Prefix = "UParticleModule";
    if (RawName.rfind(Prefix, 0) == 0)
        RawName = RawName.substr(Prefix.size());
    auto pos = RawName.find('_');
    if (pos != std::string::npos)
        RawName = RawName.substr(0, pos);

    std::string Label = RawName + DisplayName;

    if (ImGui::Selectable(Label.c_str(), isSelected))
    {
        SelectedEmitter = Emitter;
        SelectedModule = Module;
    }

}
