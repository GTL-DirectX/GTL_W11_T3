#include "EditorPanel.h"

ImVec2 UEditorPanel::GetClientSizeAsImVec2()
{
    if (!Handle)
    {
        return ImVec2(0, 0);
    }
    RECT rect;
    if (GetClientRect(Handle, &rect))
    {
        return ImVec2(static_cast<float>(rect.right - rect.left), static_cast<float>(rect.bottom - rect.top));
    }
    return ImVec2(0, 0);
}
