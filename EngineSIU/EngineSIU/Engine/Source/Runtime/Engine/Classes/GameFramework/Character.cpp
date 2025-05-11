#include "Character.h"
#include "Components/CapsuleComponent.h"

ACharacter::ACharacter()
{
    CapsuleComponent = FObjectFactory::ConstructObject<UCapsuleComponent>(this);
    RootComponent = CapsuleComponent;
}
