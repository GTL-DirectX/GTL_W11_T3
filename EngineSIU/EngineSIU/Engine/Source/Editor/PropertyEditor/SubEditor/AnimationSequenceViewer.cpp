#include "AnimationSequenceViewer.h"

#include "Components/Mesh/SkeletalMesh.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimData/AnimDataModel.h"
#include "Contents/Actors/ItemActor.h"
#include "Engine/EditorEngine.h"
#include "Engine/FFbxLoader.h"
#include "ImGui/imgui_neo_sequencer.h"

// UI Sample
//https://dev.epicgames.com/documentation/ko-kr/unreal-engine#%EC%95%A0%EB%8B%88%EB%A9%94%EC%9D%B4%EC%85%98%EC%8B%9C%ED%80%80%EC%8A%A4%ED%8E%B8%EC%A7%91%ED%95%98%EA%B8%B0

void AnimationSequenceViewer::Render()
{
    ImVec2 WinSize = ImVec2(Width, Height);

    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (Engine)
    {
        for (auto Actor : Engine->GetPreviewWorld(GEngineLoop.AnimationViewerAppWnd)->GetActiveLevel()->Actors)
        {
            if (Actor && Actor->IsA<AItemActor>())
            {
                SelectedSkeletalMeshComponent = Cast<AItemActor>(Actor)->GetComponentByClass<USkeletalMeshComponent>();
            }
        }
    }

    // Update CurrentFrameSeconds if playing
    if (bIsPlaying && SelectedAnimSequence && SelectedAnimSequence->GetDataModel())
    {
        float deltaTime = ImGui::GetIO().DeltaTime;
        CurrentFrameSeconds += deltaTime;

        // Todo:
        // const UAnimDataModel* dataModel = SelectedAnimSequence->GetDataModel();
        
        if (CurrentFrameSeconds >= MaxFrameSeconds)
        {
            if (bIsRepeating)
            {
                CurrentFrameSeconds = fmodf(CurrentFrameSeconds, MaxFrameSeconds);
                if (SelectedSkeletalMeshComponent)
                {
                    SelectedSkeletalMeshComponent->PlayAnimation(SelectedAnimSequence, bIsRepeating);
                }
            }
            else
            {
                CurrentFrameSeconds = MaxFrameSeconds;
                bIsPlaying = false; // Stop playing
            }
        }
    }

    
    /* Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar;

    /* Animation Sequencer */
    ImGui::SetNextWindowPos(ImVec2(0, WinSize.y * 0.7f));
    ImGui::SetNextWindowSize(ImVec2(WinSize.x * 0.8f - 5.0f, WinSize.y * 0.3f));
    
    ImGui::Begin("Animation Sequencer", nullptr, PanelFlags);

    RenderPlayController(ImGui::GetContentRegionAvail().x, 32.f);
    RenderAnimationSequence(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
    
    ImGui::End();

    /* Asset Details */
    ImGui::SetNextWindowPos(ImVec2(WinSize.x * 0.8f, 0));
    ImGui::SetNextWindowSize(ImVec2(WinSize.x * 0.2f, WinSize.y * 0.7f - 5.0f));

    ImGui::Begin("Asset Details", nullptr, PanelFlags);

    RenderAssetDetails();
    
    ImGui::End();


    /* Asset Browser */
    ImGui::SetNextWindowPos(ImVec2(WinSize.x * 0.8f, WinSize.y * 0.7f));
    ImGui::SetNextWindowSize(ImVec2(WinSize.x * 0.2f, WinSize.y * 0.3f));
    
    ImGui::Begin("Asset Browser", nullptr, PanelFlags);

    RenderAssetBrowser();
    
    ImGui::End();
}

void AnimationSequenceViewer::OnResize(HWND hWnd)
{
    if (hWnd != Handle)
    {
        return;
    }
    
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = static_cast<FLOAT>(ClientRect.right - ClientRect.left);
    Height = static_cast<FLOAT>(ClientRect.bottom - ClientRect.top);
}

void AnimationSequenceViewer::RenderAnimationSequence(float InWidth, float InHeight)
{
    static bool transformOpen = false;
    std::vector<ImGui::FrameIndexType> Keys;

    // auto Notifies = SelectedAnimSequence->GetNotifies();
    // for (auto&& v: Notifies)
    // {
    //     Keys.push_back(v.TriggerTime);
    // }
    
    if (ImGui::BeginNeoSequencer("Sequencer", &CurrentFrameSeconds, &StartFrameSeconds, &EndFrameSeconds, {InWidth, InHeight},
                                 ImGuiNeoSequencerFlags_EnableSelection |
                                 ImGuiNeoSequencerFlags_AllowLengthChanging |
                                 ImGuiNeoSequencerFlags_Selection_EnableDeletion))
    {
        EndFrameSeconds = std::min(EndFrameSeconds, MaxFrameSeconds);

        if (ImGui::BeginNeoGroup("Notifies", &transformOpen))
        {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            {
                UE_LOG(ELogLevel::Warning, "RIGHT CLICKED");
                ImGui::OpenPopup("TrackPopup");
            }
            
            if (ImGui::BeginPopup("TrackPopup"))
            {
                UE_LOG(ELogLevel::Warning, "OPENED POPUP");
                ImGui::Text("Track Popup");
                ImGui::EndPopup();
            }
            
            if (ImGui::BeginNeoTimelineEx("1"))
            {
                for (auto&& v: Keys)
                {
                    ImGui::NeoKeyframe(&v);
                    // Per keyframe code here
                }
                
                ImGui::EndNeoTimeLine();
            }
            ImGui::EndNeoGroup();
        }

        ImGui::EndNeoSequencer();
    }
}

void AnimationSequenceViewer::RenderPlayController(float InWidth, float InHeight)
{
    const ImGuiIO& IO = ImGui::GetIO();
    ImFont* IconFont = IO.Fonts->Fonts.size() == 1 ? IO.FontDefault : IO.Fonts->Fonts[FEATHER_FONT];
    constexpr ImVec2 IconSize = ImVec2(32, 32);
    
    ImGui::BeginChild("PlayController", ImVec2(InWidth, InHeight), ImGuiChildFlags_AutoResizeX, ImGuiWindowFlags_NoMove);
    ImGui::PushFont(IconFont);

    if (ImGui::Button("\ue9d2", IconSize)) // Rewind
    {
        CurrentFrameSeconds = 0.0f;
        bIsPlaying = false;
    }
    
    ImGui::SameLine();

    const char* PlayIcon = bIsPlaying ? "\ue99c" : "\ue9a8";
    if (ImGui::Button(PlayIcon, IconSize)) // Play & Stop
    {
        if (SelectedAnimIndex == -1 || SelectedAnimName.IsEmpty() || SelectedAnimSequence == nullptr)
        {
            ImGui::PopFont();
            ImGui::EndChild();
            return;
            
        }
        
        bIsPlaying = !bIsPlaying;

        if (bIsPlaying)
        {
            // Play
            if (CurrentFrameSeconds >= MaxFrameSeconds && MaxFrameSeconds > 0.0f)
            {
                CurrentFrameSeconds = 0.0f;
            }
            SelectedSkeletalMeshComponent->PlayAnimation(SelectedAnimSequence, bIsRepeating);
        }
        else
        {
            // Pause
        }
    }

    ImGui::SameLine();

    if (ImGui::Button("\ue96a", IconSize)) // Fast-forward
    {
        if (SelectedAnimSequence && SelectedAnimSequence->GetDataModel())
        {
            CurrentFrameSeconds = MaxFrameSeconds;
        }
        
        if (!bIsRepeating)
        {
            bIsPlaying = false; // Stop Play
        }
    }

    ImGui::SameLine();
    
    RepeatButton(&bIsRepeating);
    
    ImGui::PopFont();

    ImGui::SameLine();

    
    // Slider and Text display
    if (SelectedAnimSequence && SelectedAnimSequence->GetDataModel())
    {
        const UAnimDataModel* dataModel = SelectedAnimSequence->GetDataModel();
        int totalFrames = dataModel->NumberOfFrames > 0 ? dataModel->NumberOfFrames : 1; // Avoid 0 total frames for slider
        
        // Calculate current frame from CurrentFrameSeconds
        int currentFrameInt = 0;
        if (dataModel->FrameRate.AsDecimal() > 0) {
            currentFrameInt = static_cast<int>(round(CurrentFrameSeconds * dataModel->FrameRate.AsDecimal()));
        }
        
        int sliderMax = (dataModel->NumberOfFrames > 0) ? dataModel->NumberOfFrames -1 : 0;
        currentFrameInt = std::max(0, std::min(currentFrameInt, sliderMax));


        if (ImGui::SliderInt("##FrameSlider", &currentFrameInt, 0, sliderMax ))
        {
            if (dataModel->FrameRate.AsDecimal() > 0)
            {
                CurrentFrameSeconds = static_cast<float>(currentFrameInt) / dataModel->FrameRate.AsDecimal();
                CurrentFrameSeconds = std::min(CurrentFrameSeconds, MaxFrameSeconds); // Clamp to max length
                CurrentFrameSeconds = std::max(0.0f, CurrentFrameSeconds);           // Ensure non-negative

                // If scrubbing while playing, might want to update animation component
                // if (bIsPlaying && SelectedSkeletalMeshComponent)
                // {
                // }
            }
        }

        ImGui::SameLine();
    
        float animPercentage = (MaxFrameSeconds > 0.0f) ? (CurrentFrameSeconds / MaxFrameSeconds) * 100.0f : 0.0f;
        int displayCurrentFrame = currentFrameInt; 
        if(dataModel->NumberOfFrames > 0 && CurrentFrameSeconds >= MaxFrameSeconds && !bIsPlaying && !bIsRepeating) {
            displayCurrentFrame = dataModel->NumberOfFrames > 0 ? dataModel->NumberOfFrames -1 : 0; // Show last frame if at end and stopped
        }


        ImGui::Text("Time: %.2f/%.2fs | Frame: %d/%d | %.1f%%",
                    CurrentFrameSeconds,
                    MaxFrameSeconds,
                    displayCurrentFrame, // Display 0-indexed or 1-indexed based on preference, here 0-indexed
                    dataModel->NumberOfFrames > 0 ? dataModel->NumberOfFrames -1: 0, // Max frame index
                    animPercentage);
    }
    else
    {
        static int dummyFrame = 0;
        ImGui::SliderInt("##FrameSlider", &dummyFrame, 0, 100);
        ImGui::SameLine();
        ImGui::Text("Time: 0.00/0.00s | Frame: 0/0 | 0.0%%");
    }

    ImGui::EndChild();
}

void AnimationSequenceViewer::RenderAssetDetails() const
{
    
}

void AnimationSequenceViewer::RenderAssetBrowser()
{
    TArray<FString> animNames;
    {
        std::lock_guard<std::mutex> lock(FFbxLoader::AnimMapMutex);
        for (auto const& [name, entry] : FFbxLoader::AnimMap)
        {
            if (entry.State == FFbxLoader::LoadState::Completed && entry.Sequence != nullptr)
            {
                animNames.Add(name);
            }
        }
    }
    const char* preview_value = (SelectedAnimIndex != -1 && SelectedAnimIndex < animNames.Num()) ? *animNames[SelectedAnimIndex] : "None";

    if (ImGui::BeginCombo("Animations", preview_value))
    {
        for (int i = 0; i < animNames.Num(); ++i)
        {
            const bool is_selected = (SelectedAnimIndex == i);
            if (ImGui::Selectable(*animNames[i], is_selected))
            {
                SelectedAnimIndex = i;
                SelectedAnimName = animNames[i]; 

                UAnimSequence* newSequence = FFbxLoader::GetAnimSequenceByName(SelectedAnimName);
                if (newSequence != SelectedAnimSequence) // If sequence changed
                {
                    SelectedAnimSequence = newSequence;
                    bIsPlaying = false; // Stop playback for new animation
                    CurrentFrameSeconds = 0.0f; // Reset time
                    StartFrameSeconds = 0.0f;
                    if (SelectedAnimSequence && SelectedAnimSequence->GetDataModel())
                    {
                        MaxFrameSeconds = SelectedAnimSequence->GetDataModel()->PlayLength;
                        EndFrameSeconds = MaxFrameSeconds; // Set sequencer end to anim length
                    }
                    else
                    {
                        MaxFrameSeconds = 0.0f;
                        EndFrameSeconds = 0.0f;
                    }
                }
            }
            if (is_selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

}

void AnimationSequenceViewer::PlayButton(bool* v) const
{
    // Not implement
}

void AnimationSequenceViewer::RepeatButton(bool* v) const
{
    ImVec4 ColorBg = *v ? ImVec4(0.0f, 0.3f, 0.0f, 1.0f) : ImVec4(0, 0, 0, 1.0f);
    
    ImGui::PushStyleColor(ImGuiCol_Button, ColorBg);
    if (ImGui::Button("\ue9d1", ImVec2(32, 32))) // Repeat
    {
        *v = !*v;
    }
    ImGui::PopStyleColor();
}
