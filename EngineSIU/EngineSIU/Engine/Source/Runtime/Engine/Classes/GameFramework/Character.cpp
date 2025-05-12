#include "Character.h"

#include "SoundManager.h"
#include "Animation/AnimTypes.h"
#include "Components/CapsuleComponent.h"

ACharacter::ACharacter()
{
    CapsuleComponent = FObjectFactory::ConstructObject<UCapsuleComponent>(this);
    AddComponent<UCapsuleComponent>(FName("UCapsuleComponent_0"));
    RootComponent = CapsuleComponent;
}

void ACharacter::HandleAnimNotify(const FAnimNotifyEvent& NotifyEvent) const 
{
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

