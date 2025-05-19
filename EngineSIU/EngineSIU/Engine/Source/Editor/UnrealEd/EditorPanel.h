#pragma once


#ifndef __ICON_FONT_INDEX__

#define __ICON_FONT_INDEX__
#define DEFAULT_FONT        0
#define FEATHER_FONT        1

#endif // !__ICON_FONT_INDEX__

#include "Windows.h"
#include "ImGui/imgui.h"

struct ImGuiContext;

class UEditorPanel
{
public:
    virtual ~UEditorPanel() = default;
    virtual void Init() {}
    virtual void Render() = 0;
    virtual void OnResize(HWND hWnd) = 0;

    ImVec2 GetClientSizeAsImVec2();
    
public:
    HWND Handle = nullptr;
    ImGuiContext* GuiContext = nullptr;


};

