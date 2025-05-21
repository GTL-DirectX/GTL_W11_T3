#include "ParticleLODLevel.h"

#include "ParticleModule.h"
#include "UObject/ObjectFactory.h"
#include "UObject/Casts.h"

// TODO : 현재 미사용인 함수, 추후 PostLoad() 등에서 호출 필요
// SpawnModules  :FParticleEmitterInstance::SpawnParticles 에서 쓰임
// UpdateModules :FParticleEmitterInstance::Tick_ModuleUpdate에서 쓰임
void UParticleLODLevel::UpdateModuleLists()
{
    UParticleModule* Module;
    int32 TypeDataModuleIndex = -1;

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
