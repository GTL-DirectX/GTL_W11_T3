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
#include "Engine/FObjLoader.h"
#include "Particles/TypeData/ParticleModuleTypeDataSprite.h"
#include "Particles/TypeData/ParticleModuleTypeDataMesh.h"

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
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoResize |
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
        "Save System", // ← OpenPopup의 ID와 정확히 일치해야 함
        nullptr,       // nullptr 주면 X 버튼은 생기지 않지만, Cancel 버튼으로만 닫음
        ImGuiWindowFlags_AlwaysAutoResize
    ))
    {
        static char SaveSystemName[128] = "";

        ImGui::Text("Enter name for this particle system:");
        ImGui::PushItemWidth(200);
        ImGui::InputText("##SaveSysName", SaveSystemName, IM_ARRAYSIZE(SaveSystemName));
        ImGui::PopItemWidth();

        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(100, 0)))
        {
            UE_LOG(
                ELogLevel::Warning, TEXT("CurrentParticleSystem is %s and name is %s"),
                CurrentParticleSystem != nullptr ? TEXT("Valid") : TEXT("Null"), SaveSystemName
            );

            // SaveSystemName 을 이용해 실제 저장 처리
            FString Key(SaveSystemName);

            UParticleSystem* SystemPtr = Cast<UParticleSystem>(CurrentParticleSystem->Duplicate(CurrentParticleSystem->GetOuter()));
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
        if (ImGui::Selectable("New Particle Sprite Emitter"))
        {
            CurrentParticleSystem->AddNewEmitterSprite();

            ImGui::CloseCurrentPopup();
        }
        if (ImGui::Selectable("New Particle Mesh Emitter"))
        {
            CurrentParticleSystem->AddNewEmitterMesh();

            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (!CurrentParticleSystemComponent || !CurrentParticleSystem)
    {
        ImGui::End();
        return;
    }

    // Emitters 그리기
    auto& Emitters = CurrentParticleSystem->Emitters;
    float EmitterWidth = 220.0f;
    float totalContentWidth = EmitterWidth * float(Emitters.Num());
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

        // Material만 하드코딩으로 적용
        if (auto* RequiredModule = Cast<UParticleModuleRequired>(SelectedModule))
        {
            UMaterial* CurrentMat = RequiredModule->Material;
            FString CurrentName = CurrentMat
                ? CurrentMat->GetMaterialInfo().MaterialName
                : FString(TEXT("None"));
            std::string CurrentNameUtf8 = GetData(*CurrentName);

            auto& MatMap = FObjManager::GetMaterials();
            std::vector<FString> KeysToRemove;
            KeysToRemove.reserve(MatMap.Num());
            for (auto& Pair : MatMap)
            {
                UMaterial* Mat = Pair.Value;
                const auto& MI = Mat->GetMaterialInfo();
                // Diffuse 플래그가 없으면 제거 대상
                if ((MI.TextureFlag & static_cast<uint16>(EMaterialTextureFlags::MTF_Diffuse)) == 0)
                {
                    KeysToRemove.push_back(Pair.Key);
                }
            }
            for (auto& Key : KeysToRemove)
            {
                MatMap.Remove(Key);
            }
            
            std::vector<FString> MatNames;
            MatNames.reserve(MatMap.Num());
            for (auto& Pair : MatMap)
            {
                MatNames.push_back(Pair.Key);
            }

            int CurrentIdx = 0;
            for (int i = 0; i < (int)MatNames.size(); ++i)
            {
                if (MatNames[i] == CurrentName)
                {
                    CurrentIdx = i;
                    break;
                }
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();
            
            ImGui::Text("[ Material ]");
            ImGui::Spacing();

            float comboW = ImGui::CalcItemWidth();
            ImGui::SetNextWindowSizeConstraints(
                ImVec2(comboW, 0),
                ImVec2(comboW, FLT_MAX)
            );

            float comboImgSize = 50.0f;
            if (ImGui::BeginCombo("##MaterialCombo", CurrentNameUtf8.c_str()))
            {
                ImGui::BeginChild("##MaterialScrollRegion", ImVec2(0, 200), /*border=*/false);
                float rowH = comboImgSize;
                
                for (int i = 0; i < MatNames.size(); ++i)
                {
                    const FString& MatName = MatNames[i];
                    UMaterial* Mat = FObjManager::GetMaterial(MatName);
                    FMaterialInfo& MaterialInfo = Mat->GetMaterialInfo();

                    ImTextureID texID;
                    if ( (MaterialInfo.TextureFlag & static_cast<uint16>(EMaterialTextureFlags::MTF_Diffuse)) != 0 )
                    {
                        int slot = static_cast<int>(EMaterialTextureSlots::MTS_Diffuse);
                        if (MaterialInfo.TextureInfos.IsValidIndex(slot))
                        {
                            auto& TI = MaterialInfo.TextureInfos[slot];
                            if (auto ptr = FEngineLoop::ResourceManager.GetTexture(TI.TexturePath))
                                texID = (ImTextureID)ptr->TextureSRV;
                        }
                    }

                    ImGui::PushID(i);

                    float yStart = ImGui::GetCursorPosY();
                    bool isSelected = (i == CurrentIdx);

                    std::string nameUtf8 = GetData(*MatName);
                    ImVec2 ts = ImGui::CalcTextSize(nameUtf8.c_str());
                    ImGuiStyle& style = ImGui::GetStyle();
                    float padX = style.FramePadding.x;
                    float padY = (rowH - ts.y) * 0.5f;

                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(padX, padY));
                    bool clicked = ImGui::Selectable(
                        "",                                       // 라벨은 비워두고
                        isSelected,
                        ImGuiSelectableFlags_SpanAvailWidth,     // 남은 가로 전체 차지
                        ImVec2(0, rowH)                     // 높이만 rowH 고정
                    );
                    ImGui::PopStyleVar();

                    if (clicked)
                    {
                        RequiredModule->Material = Mat;
                        CurrentIdx = i;
                    }

                    ImVec2 itemMin = ImGui::GetItemRectMin();
                    // 이미지
                    ImGui::SetCursorScreenPos(ImVec2(itemMin.x + 10.0f, itemMin.y + 2.5f));
                    if (texID)
                        ImGui::Image(texID, ImVec2(rowH, rowH));
                    else
                        ImGui::Dummy(ImVec2(rowH, rowH));

                    // 텍스트: 이미지 오른쪽 + 4px 여백, 수직 오프셋 padY
                    ImGui::SetCursorScreenPos(ImVec2(itemMin.x + rowH + 30.0f, itemMin.y + padY));
                    ImGui::Text("%s", nameUtf8.c_str());
                    ImGui::PopID();
                    ImGui::SetCursorPosY( yStart + rowH + 8.0f );
                }
                ImGui::EndChild();
                ImGui::EndCombo();
            }
            
            ImGui::SameLine(0.0f, 30.0f);
            
            float comboH = ImGui::GetFrameHeight();
            const float imgSize = 64.0f;
            float offsetY = (comboH - imgSize) * 0.5f;
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() + offsetY);
            
            if (UMaterial* SelMat = RequiredModule->Material)
            {
                FMaterialInfo& SelInfo = SelMat->GetMaterialInfo();
                const uint16 DiffuseFlag = static_cast<uint16>(EMaterialTextureFlags::MTF_Diffuse);

                if ((SelInfo.TextureFlag & DiffuseFlag) != 0)
                {
                    const int Slot = static_cast<int>(EMaterialTextureSlots::MTS_Diffuse);
                    if (SelInfo.TextureInfos.IsValidIndex(Slot))
                    {
                        const FTextureInfo& TI = SelInfo.TextureInfos[Slot];
                        auto TexPtr = FEngineLoop::ResourceManager.GetTexture(TI.TexturePath);
                        if (TexPtr && TexPtr->TextureSRV)
                        {
                            ImGui::Image((ImTextureID)TexPtr->TextureSRV, ImVec2(imgSize, imgSize));
                        }
                        else
                        {
                            ImGui::Text("Diffuse texture not found or not loaded.");
                        }

                        // 테두리 그리기
                        ImVec2 pMin = ImGui::GetItemRectMin();
                        ImVec2 pMax = ImGui::GetItemRectMax();
                        ImGui::GetWindowDrawList()->AddRect(
                            pMin,
                            pMax,
                            IM_COL32(100, 100, 100, 50),
                            0.0f,    // rounding
                            ImDrawFlags_RoundCornersAll,
                            1.0f     // thickness
                        );
                    }
                }
            }
            ImGui::Separator();
            ImGui::Spacing();
        }
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
