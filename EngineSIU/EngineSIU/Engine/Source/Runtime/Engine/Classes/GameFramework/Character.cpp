#include "Character.h"

#include "SoundManager.h"
#include "Animation/AnimTypes.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"

ACharacter::ACharacter()
{
    
}

void ACharacter::PostSpawnInitialize()
{
    Super::PostSpawnInitialize();

    AddComponent<UCapsuleComponent>(FName("CapsuleComponent_0"));
    RootComponent = CapsuleComponent;

    Mesh = AddComponent<USkeletalMeshComponent>(FName("SkeletalMeshComponent_0"));
    Mesh->SetupAttachment(CapsuleComponent);
}

void ACharacter::HandleAnimNotify(const FAnimNotifyEvent& NotifyEvent) const 
{
    if (NotifyEvent.NotifyName == FName("NONE"))
    {
        FSoundManager::GetInstance().PlaySound("sizzle");
        return;
    }
    else if (NotifyEvent.NotifyName == FName("WALK"))
    {
        FSoundManager::GetInstance().PlaySound("fishdream");
        return;
    }
    //switch (NotifyEvent.NotifyName)
    //{
    //case FName("FIRE"):
    //    FSoundManager::GetInstance().PlaySound("sizzle");
    //    break;
    //case FName("EXPLOSION"):
    //    FSoundManager::GetInstance().PlaySound("fishdream");
    //    break;
    //default:
    //    break;
    //}
}

