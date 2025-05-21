#include "ParticleSystem.h"

#include "ParticleEmitter.h"
#include "ParticleLODLevel.h"
#include "UObject/ObjectFactory.h"
#include "UObject/Casts.h"

#include "TypeData/ParticleModuleTypeDataSprite.h"
#include "TypeData/ParticleModuleTypeDataMesh.h"

void UParticleSystem::PostInitProperties()
{
    Super::PostInitProperties();
    // Emitters 초기화
    Emitters.Empty();
    Emitters.Add(FObjectFactory::ConstructObject<UParticleEmitter>(this));

    BuildEmitters();
}

UObject* UParticleSystem::Duplicate(UObject* InOuter)
{
    UParticleSystem* NewParticleSystem = Cast<UParticleSystem>(Super::Duplicate(InOuter));
    if (NewParticleSystem)
    {
        NewParticleSystem->Emitters.SetNum(Emitters.Num());
        
        // Emitters 복사
        for (int32 i = 0; i < Emitters.Num(); ++i)
        {
            if (Emitters[i])
            {
                NewParticleSystem->Emitters[i] = Cast<UParticleEmitter>(Emitters[i]->Duplicate(NewParticleSystem));
            }
        }
    }
    return NewParticleSystem;
}

void UParticleSystem::BuildEmitters()
{
    const int32 EmitterCount = Emitters.Num();
    for (int32 EmitterIndex = 0; EmitterIndex < EmitterCount; ++EmitterIndex)
    {
        if (UParticleEmitter* Emitter = Emitters[EmitterIndex])
        {
            Emitter->Build();
        }
    }
}

void UParticleSystem::AddNewEmitterSprite()
{
    UParticleEmitter* NewEmitter = FObjectFactory::ConstructObject<UParticleEmitter>(this);
    Emitters.Add(NewEmitter);
    NewEmitter->Build();
    NewEmitter->EmitterName = FName("SpriteEmitter_" + std::to_string(Emitters.Num()));
    NewEmitter->LODLevels[0]->bSpriteTypeData = true;
}

void UParticleSystem::AddNewEmitterMesh()
{
    UParticleEmitter* NewEmitter = FObjectFactory::ConstructObject<UParticleEmitter>(this);
    Emitters.Add(NewEmitter);
    NewEmitter->Build();
    NewEmitter->EmitterName = FName("MeshEmitter_" + std::to_string(Emitters.Num()));
    UParticleModuleTypeDataMesh* TypeDataModule = FObjectFactory::ConstructObject<UParticleModuleTypeDataMesh>(NewEmitter);
    if (NewEmitter->LODLevels[0]->TypeDataModule)
    {
        NewEmitter->LODLevels[0]->TypeDataModule = nullptr;
    }
    NewEmitter->LODLevels[0]->TypeDataModule = TypeDataModule;
    NewEmitter->LODLevels[0]->bSpriteTypeData = false;
}
