#include "ImGuiInspector.h"

#include "Components/SceneComponent.h"
#include "UObject/Class.h"
#include "ImGui/ImGui.h"
#include "UnrealEd/ImGuiWidget.h"
#include "UObject/Casts.h"

namespace ImGuiInspector
{
    void DrawFieldEditor(UField* Field, UObject*& ObjPtr)
    {
        if ((Field->Flags & EditAnywhere) == 0)
            return;

        switch (Field->PropType)
        {
        case EPropertyType::Int32:
        {
            auto* FI = static_cast<TField<int32>*>(Field);
            int32  Val = FI->GetValue(ObjPtr);
            if (ImGui::DragInt(*Field->Name, &Val, 1.0f))
                FI->SetValue(ObjPtr, Val);
            break;
        }
        case EPropertyType::Float:
        {
            auto* FF = static_cast<TField<float>*>(Field);
            float  Val = FF->GetValue(ObjPtr);
            if (ImGui::DragFloat(*Field->Name, &Val, 0.1f))
                FF->SetValue(ObjPtr, Val);
            break;
        }
        case EPropertyType::Bool:
        {
            auto* FB = static_cast<TField<bool>*>(Field);
            bool   Val = FB->GetValue(ObjPtr);
            if (ImGui::Checkbox(*Field->Name, &Val))
                FB->SetValue(ObjPtr, Val);
            break;
        }
        case EPropertyType::Struct:
        {
            break;
        }
        case EPropertyType::Vector:
        {
            TField<FVector>* FV = dynamic_cast<TField<FVector>*>(Field);
            FVector Vec = FV->GetValue(ObjPtr);
            FImGuiWidget::DrawVec3Control(*Field->Name, Vec, 0, 85);
            FV->SetValue(ObjPtr, Vec);
            if (auto* SC = Cast<USceneComponent>(ObjPtr))
            {
                if (Field->Name == TEXT("RelativeLocation"))
                {
                    SC->SetRelativeLocation(Vec);
                }
                else if (Field->Name == TEXT("RelativeScale3D"))
                {
                    SC->SetRelativeScale3D(Vec);
                }
            }
            break;
        }
        case EPropertyType::Rotator:
        {
            TField<FRotator>* FR = dynamic_cast<TField<FRotator>*>(Field);
            FRotator R = FR->GetValue(ObjPtr);
            FImGuiWidget::DrawRot3Control(*Field->Name, R, 0, 85);
            FR->SetValue(ObjPtr, R);
            if (auto* SC = Cast<USceneComponent>(ObjPtr))
            {
                SC->SetRelativeRotation(R);
            }
            break;
        }
        default:
            ImGui::Text("%s: (unsupported)", *Field->Name);
            break;
        }
    }
}

