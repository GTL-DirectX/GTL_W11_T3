#pragma once
#include "Core/HAL/PlatformType.h"
#include "Engine/ResourceMgr.h"
#include "LevelEditor/SlateAppMessageHandler.h"
#include "Renderer/Renderer.h"
#include "UnrealEd/PrimitiveDrawBatch.h"
#include "Stats/ProfilerStatsManager.h"
#include "Stats/GPUTimingManager.h"


class UWorld;
class FGraphicDevice;
class FDXDBufferManager;
class UnrealEd;
class SSplitterV;
class SSplitterH;
class SLevelEditor;
class SlateViewer;
class FEditorViewportClient;
class FSlateAppMessageHandler;
class UImGuiManager;

class FEngineLoop
{
public:
    FEngineLoop();

    int32 PreInit();
    int32 Init(HINSTANCE hInstance);
    void Render() const;
    void Render(HWND Handle) const;
    void Tick();
    void Exit();

    void GetClientSize(HWND hWnd, uint32& OutWidth, uint32& OutHeight) const;

private:
    HWND CreateNewWindow(HINSTANCE hInstance, const WCHAR* WindowClass, const WCHAR* WindowName, int Width, int Height, HWND Parent = nullptr) const;
    
    static LRESULT CALLBACK AppWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

    void UpdateUI(HWND hWnd, bool bSubWindow = false) const;

public:
    static FGraphicsDevice GraphicDevice;
    static FRenderer Renderer;
    static UPrimitiveDrawBatch PrimitiveDrawBatch;
    static FResourceMgr ResourceManager;
    static uint32 TotalAllocationBytes;
    static uint32 TotalAllocationCount;
    static TMap<HWND, ImGuiContext*> WndImGuiContextMap;
    
    HWND MainAppWnd;
    
    // @todo SubWindow를 여러개 만들 수 있도록 수정
    // Skeletal Mesh Viewer
    HWND SkeletalMeshViewerAppWnd;

    /** Animation Viewer Handle */
    HWND AnimationViewerAppWnd;

    // Particle System Viewer
    HWND ParticleSystemViewerAppWnd;

    void ToggleWindow(HWND hWnd) const;
    void Show(HWND HWnd) const;
    void Hide(HWND hWnd) const;

    bool IsVisible(const HWND hWnd) const { return IsWindowVisible(hWnd); }

    FGPUTimingManager GPUTimingManager;
    FGPUMemoryManager GPUMemoryManager;
    FEngineProfiler EngineProfiler;

private:
    UImGuiManager* MainUIManager;
    UImGuiManager* SkeletalMeshViewerUIManager;
    UImGuiManager* AnimationViewerUIManager;
    UImGuiManager* ParticleSystemViewerUIManager;
    ImGuiContext* CurrentImGuiContext;

    std::unique_ptr<FSlateAppMessageHandler> AppMessageHandler;
    SLevelEditor* LevelEditor;
    SlateViewer* SkeletalMeshViewer;
    SlateViewer* AnimationViewer;
    SlateViewer* ParticleSystemViewer;
    UnrealEd* UnrealEditor;
    FDXDBufferManager* BufferManager; //TODO: UEngine으로 옮겨야함.

    bool bIsExit = false;
    // @todo Option으로 선택 가능하도록
    int32 TargetFPS = 999;

public:
    SLevelEditor* GetLevelEditor() const { return LevelEditor; }
    SlateViewer* GetSkeletalMeshViewer() const { return SkeletalMeshViewer; }
    SlateViewer* GetAnimationViewer() const { return AnimationViewer; }
    SlateViewer* GetParticleSystemViewer() const { return ParticleSystemViewer; }
    UnrealEd* GetUnrealEditor() const { return UnrealEditor; }

    FSlateAppMessageHandler* GetAppMessageHandler() const { return AppMessageHandler.get(); }
    
};
