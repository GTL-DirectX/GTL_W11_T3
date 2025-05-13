#include "ImGuiInspector.h"
#include "UObject/Class.h"
#include "ImGui/ImGui.h"

namespace ImGuiInspector
{
    void DrawFieldEditor(UField* Field, UObject* ObjPtr)
    {
        switch (Field->PropType)
        {
        case EPropertyType::Int32:
        {
            auto* FI = static_cast<TField<int32>*>(Field);
            int32 Val = FI->GetValue(ObjPtr);
            if (ImGui::DragInt(*Field->Name, &Val, 1.0f))
                FI->SetValue(ObjPtr, Val);
            break;
        }
        case EPropertyType::Float:
        {
            auto* FF = static_cast<TField<float>*>(Field);
            float Val = FF->GetValue(ObjPtr);
            if (ImGui::DragFloat(*Field->Name, &Val, 0.1f))
                FF->SetValue(ObjPtr, Val);
            break;
        }
        case EPropertyType::Bool:
        {
            auto* FB = static_cast<TField<bool>*>(Field);
            bool Val = FB->GetValue(ObjPtr);
            if (ImGui::Checkbox(*Field->Name, &Val))
                FB->SetValue(ObjPtr, Val);
            break;
        }
        case EPropertyType::Struct:
            // FVector 예시
            if (auto* FVec = dynamic_cast<TField<FVector>*>(Field))
            {
                FVector Vec = FVec->GetValue(ObjPtr);
                float arr[3] = { Vec.X, Vec.Y, Vec.Z };
                if (ImGui::DragFloat3(*Field->Name, arr, 0.1f))
                    FVec->SetValue(ObjPtr, FVector{ arr[0], arr[1], arr[2] });
            }
            break;

        default:
            ImGui::Text("%s: (unsupported)", *Field->Name);
            break;
        }
    }
}

