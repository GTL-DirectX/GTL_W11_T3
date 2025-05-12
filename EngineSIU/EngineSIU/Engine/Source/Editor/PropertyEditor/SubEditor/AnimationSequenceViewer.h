#pragma once
#include "GameFramework/Actor.h"
#include "ImGui/imgui_neo_sequencer.h"
#include "UnrealEd/EditorPanel.h"

class USkeletalMeshComponent;
class UAnimSequence;

class AnimationSequenceViewer : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    void RenderAnimationSequence(float InWidth, float InHeight);
    void RenderPlayController(float InWidth, float InHeight);
    void RenderAssetDetails() const;
    void RenderAssetBrowser();

    void PlayButton(bool* v) const;
    void RepeatButton(bool* v) const;
    
private:
    float Width = 800, Height = 600;

    float CurrentFrameSeconds = 0.0f;
    float StartFrameSeconds = 0.0f;
    float EndFrameSeconds = 1.0f;
    float MaxFrameSeconds = 1.0f;

    int CurrentFrame = 0;
    int StartFrame = 0;
    int EndFrame = 0;
    
    bool bIsPlaying = false;
    bool bIsRepeating = false;

    /* Animation Property for Debug */
    int SelectedAnimIndex = -1;
    FString SelectedAnimName;

    int SelectedTrackIndex = -1;
    
    UAnimSequence* SelectedAnimSequence = nullptr;
    USkeletalMeshComponent* SelectedSkeletalMeshComponent = nullptr;
    TMap<int32, TArray<ImGui::FrameIndexType>> TrackAndKeyMap;
};
