#pragma once
#include <memory>

#include "Texture.h"
#include "Container/Map.h"

class UMaterial;
class FRenderer;
class FGraphicsDevice;
class FResourceMgr
{

public:
    void Initialize(FRenderer* renderer, FGraphicsDevice* device);
    void Release(FRenderer* renderer);
    HRESULT LoadTextureFromFile(ID3D11Device* device, const wchar_t* filename, bool bIsSRGB = true);
    HRESULT LoadTextureFromDDS(ID3D11Device* device, ID3D11DeviceContext* context, const wchar_t* filename);

    std::shared_ptr<FTexture> GetTexture(const FWString& name) const;
    bool HasMaterial(const FString& Name) const { return materialMap.Contains(Name); }
    UMaterial* GetMaterial(const FString& Name) const { return materialMap[Name]; }
    TMap<FString, UMaterial*>& GetMaterialMap() { return materialMap; }
    void AddMaterial(const FString& Name, UMaterial* Material) { materialMap[Name] = Material; }
private:
    TMap<FWString, std::shared_ptr<FTexture>> textureMap;
    TMap<FString, UMaterial*> materialMap;
};
