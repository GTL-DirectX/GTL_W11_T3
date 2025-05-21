#include "ParticleLODLevel.h"

#include "ParticleModule.h"
#include "UObject/ObjectFactory.h"
#include "UObject/Casts.h"

#include "Particles/ParticleModuleRequired.h"
#include "Particles/ParticleModuleSpawn.h"
#include "ParticleModuleSize.h"
#include "ParticleModuleLifeTime.h"
#include "ParticleModuleVelocity.h"
#include "ParticleModuleColor.h"
#include "ParticleModuleLocation.h"
#include "TypeData/ParticleModuleTypeDataMesh.h"
#include "TypeData/ParticleModuleTypeDataSprite.h"

void UParticleLODLevel::PostInitProperties()
{
    Super::PostInitProperties();
    // LODLevel 초기화
    LODLevel = 0;
    bEnabled = true;
    PeakActiveParticles = 0;
    // 모듈 초기화
    Modules.Empty();
    SpawnModules.Empty();
    UpdateModules.Empty();
    TypeDataModule = nullptr;
    RequiredModule = nullptr;
    SpawnModule = nullptr;

    // 기본 모듈 추가
    RequiredModule = FObjectFactory::ConstructObject<UParticleModuleRequired>(this);
    Modules.Add(RequiredModule);

    SpawnModule = FObjectFactory::ConstructObject<UParticleModuleSpawn>(this);
    Modules.Add(SpawnModule);

    TypeDataModule = FObjectFactory::ConstructObject<UParticleModuleTypeDataSprite>(this);

    Modules.Add(FObjectFactory::ConstructObject<UParticleModuleLocation>(this));
    Modules.Add(FObjectFactory::ConstructObject<UParticleModuleVelocity>(this));
    Modules.Add(FObjectFactory::ConstructObject<UParticleModuleLifeTime>(this));
    Modules.Add(FObjectFactory::ConstructObject<UParticleModuleSize>(this));
    Modules.Add(FObjectFactory::ConstructObject<UParticleModuleColor>(this));
}

UObject* UParticleLODLevel::Duplicate(UObject* InOuter)
{
    UParticleLODLevel* NewLODLevel = Cast<UParticleLODLevel>(Super::Duplicate(InOuter));
    if (NewLODLevel)
    {
        NewLODLevel->LODLevel = LODLevel;
        NewLODLevel->bEnabled = bEnabled;
        NewLODLevel->PeakActiveParticles = PeakActiveParticles;
        // LODLevel의 모듈 복사
        NewLODLevel->Modules.SetNum(Modules.Num());
        NewLODLevel->TypeDataModule = TypeDataModule;
        for (int32 i = 0; i < Modules.Num(); ++i)
        {
            if (Modules[i])
            {
                NewLODLevel->Modules[i] = Cast<UParticleModule>(Modules[i]->Duplicate(NewLODLevel));
            }
        }
    }
    return NewLODLevel;
}

// TODO : 현재 미사용인 함수, 추후 PostLoad() 등에서 호출 필요
// SpawnModules  :FParticleEmitterInstance::SpawnParticles 에서 쓰임
// UpdateModules :FParticleEmitterInstance::Tick_ModuleUpdate에서 쓰임
void UParticleLODLevel::UpdateModuleLists()
{
    UParticleModule* Module;
    int32 TypeDataModuleIndex = -1;

    SpawnModules.Empty();
    UpdateModules.Empty();

    for (int32 i = 0; i < Modules.Num(); i++)
    {
        Module = Modules[i];
        if (Module)
        {
            if (Module->bSpawnModule)
            {
                SpawnModules.Add(Module);
            }
            if (Module->bUpdateModule)
            {
                UpdateModules.Add(Module);
            }
        }
    }

    /* TypeDAtaModule 분기 추가 필요 (UParticleModuleTypeDataMesh에 대한 분기, 언리얼 기준) */
}

void UParticleLODLevel::AddModule(UClass* ModuleClass)
{
    if (ModuleClass)
    {
        UParticleModule* NewModule = Cast<UParticleModule>(FObjectFactory::ConstructObject(ModuleClass, this));
        if (NewModule)
        {
            Modules.Add(NewModule);
            UpdateModuleLists();
        }
    }
}

void UParticleLODLevel::AddModule(UParticleModule* Module)
{
    if (Module)
    {
        Modules.Add(Module);
        UpdateModuleLists();
    }
}

void UParticleLODLevel::RemoveModule(UParticleModule* Module)
{
    if (Module)
    {
        Modules.Remove(Module);
        UpdateModuleLists();
    }
}
