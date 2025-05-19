#include "UnrealEd.h"
#include "EditorPanel.h"

#include "PropertyEditor/ControlEditorPanel.h"
#include "PropertyEditor/OutlinerEditorPanel.h"
#include "PropertyEditor/PropertyEditorPanel.h"
#include "PropertyEditor/SubEditor/SkeletalTreePanel.h"
#include "PropertyEditor/SkeletalMeshControlPanel.h"
#include "PropertyEditor/SubEditor/AnimationSequenceViewerPanel.h"
#include <PropertyEditor/SubEditor/ParticleSystemViewerPanel.h>

void UnrealEd::Initialize()
{
    // MainAppWnd
    auto ControlPanel = std::make_shared<ControlEditorPanel>();
    ControlPanel->Handle = GEngineLoop.MainAppWnd;
    AddEditorPanel("ControlPanel", ControlPanel);

    auto OutlinerPanel = std::make_shared<OutlinerEditorPanel>();
    OutlinerPanel->Handle = GEngineLoop.MainAppWnd;
    AddEditorPanel("OutlinerPanel", OutlinerPanel);

    auto PropertyPanel = std::make_shared<PropertyEditorPanel>();
    PropertyPanel->Handle = GEngineLoop.MainAppWnd;
    AddEditorPanel("PropertyPanel", PropertyPanel);

    // SkeletalMeshViewerAppWnd
    auto SubWindowSkeletalTreePanel = std::make_shared<SkeletalTreePanel>();
    SubWindowSkeletalTreePanel->Handle = GEngineLoop.SkeletalMeshViewerAppWnd;
    AddEditorPanel("SubWindowSkeletalTreePanel", SubWindowSkeletalTreePanel, true);

    auto SkeletalControlPanel = std::make_shared<SkeletalMeshControlPanel>();
    SkeletalControlPanel->Handle = GEngineLoop.SkeletalMeshViewerAppWnd;
    AddEditorPanel("SkeletalControlPanel", SkeletalControlPanel, true);

    // AnimationViewerAppWnd
    auto AnimationSequencePanel = std::make_shared<AnimationSequenceViewerPanel>();
    AnimationSequencePanel->Handle = GEngineLoop.AnimationViewerAppWnd;
    AddEditorPanel("AnimationSequencePanel", AnimationSequencePanel, true);

    // ParticleSystemViewerAppWnd
    auto PropertyPanelq = std::make_shared<ParticleSystemViewerPanel>();
    PropertyPanelq->Handle = GEngineLoop.ParticleSystemViewerAppWnd;
    AddEditorPanel("ParticleSystemViewerPanel", PropertyPanelq, true);

    Init();
}

void UnrealEd::Init() const
{
    for (const auto& Panel : Panels)
    {
        Panel.Value->Init();
    }
}

void UnrealEd::Render() const
{
    for (const auto& Panel : Panels)
    {
        Panel.Value->Render();
    }
}

void UnrealEd::RenderSubWindowPanel(HWND hWnd) const
{
    for (const auto& Panel : SubPanels)
    {
        if (Panel.Value->Handle != hWnd)
        {
            continue;
        }
        Panel.Value->Render();
    }
}

void UnrealEd::AddEditorPanel(const FString& PanelId, const std::shared_ptr<UEditorPanel>& EditorPanel, bool bSubWindow)
{
    (bSubWindow ? SubPanels : Panels)[PanelId] = EditorPanel;
}

void UnrealEd::OnResize(HWND hWnd, bool bSubWindow) const
{
    auto& targetPanels = bSubWindow ? SubPanels : Panels;

    for (auto& PanelPair : targetPanels)
    {
        if (PanelPair.Value)
        {
            PanelPair.Value->OnResize(hWnd);
        }
    }
}

std::shared_ptr<UEditorPanel> UnrealEd::GetEditorPanel(const FString& PanelId)
{
    return Panels[PanelId];
}

std::shared_ptr<UEditorPanel> UnrealEd::GetSubEditorPanel(const FString& PanelId)
{
    return SubPanels[PanelId];
}
