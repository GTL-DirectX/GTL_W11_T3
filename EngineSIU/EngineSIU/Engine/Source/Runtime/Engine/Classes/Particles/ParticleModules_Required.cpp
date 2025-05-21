#include "ParticleEmitterInstances.h"
#include "ParticleModuleRequired.h"
#include "UObject/ObjectFactory.h"

UParticleModuleRequired::UParticleModuleRequired()
{
    bEnabled = true;
    bSpawnModule = true;
    bUpdateModule = false;
}

void UParticleModuleRequired::PostInitProperties()
{
    Super::PostInitProperties();
    
    EmitterDuration = 3.0f;
    SpawnRate = 10.0f / EmitterDuration;
    Material = CreateDefaultMaterial();
}

void UParticleModuleRequired::Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase)
{
    Super::Spawn(Owner, Offset, SpawnTime, ParticleBase);
    
    // TODO : Material Setting 방식 더 나은 방식 생각해봐야함
    if (!Owner->Material)
        Owner->Material = Material;
}

UMaterial* UParticleModuleRequired::CreateDefaultMaterial()
{
    FString DefaultParticleName = TEXT("DefaultParticle");
    UMaterial* DefaultParticleMat = nullptr;
    if (FEngineLoop::ResourceManager.HasMaterial(DefaultParticleName))
    {
        DefaultParticleMat = FEngineLoop::ResourceManager.GetMaterial(DefaultParticleName);
    }
    else
    {
        DefaultParticleMat = FObjectFactory::ConstructObject<UMaterial>(nullptr);
        DefaultParticleMat->GetMaterialInfo().MaterialName = DefaultParticleMat->GetName();
        int reservedCount = static_cast<uint32>(EMaterialTextureSlots::MTS_MAX);
        for (int i = 0; i < reservedCount; ++i)
            DefaultParticleMat->GetMaterialInfo().TextureInfos.Add({});
    
        const wchar_t* TextureFilePath = L"Assets/Texture/Star.png";
        if (!FEngineLoop::ResourceManager.GetTexture(TextureFilePath))
            FEngineLoop::ResourceManager.LoadTextureFromFile(FEngineLoop::GraphicDevice.Device, TextureFilePath);
        
        const uint32 SlotIdx = static_cast<uint32>(EMaterialTextureSlots::MTS_Diffuse);
        FTextureInfo& Slot = DefaultParticleMat->GetMaterialInfo().TextureInfos[SlotIdx];
        Slot.TextureName = TEXT(TextureFilePath);
        Slot.TexturePath = TEXT(TextureFilePath);
        Slot.bIsSRGB = true;
        DefaultParticleMat->GetMaterialInfo().TextureFlag |= static_cast<uint16>(EMaterialTextureFlags::MTF_Diffuse);
        
        FEngineLoop::ResourceManager.AddMaterial(DefaultParticleName, DefaultParticleMat);
    }
    return DefaultParticleMat;
}
