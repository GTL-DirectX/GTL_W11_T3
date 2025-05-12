#include "Character.h"
#include "Components/CapsuleComponent.h"

ACharacter::ACharacter()
{
    CapsuleComponent = FObjectFactory::ConstructObject<UCapsuleComponent>(this);
    AddComponent<UCapsuleComponent>(FName("UCapsuleComponent_0"));
    RootComponent = CapsuleComponent;
}
