#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include "Core/Misc/Spinlock.h"

enum class EAssetType : uint8
{
    StaticMesh,
    SkeletalMesh,
    Texture2D,
    Material,
};

struct FAssetInfo
{
    enum class LoadState : int8
    {
        Loading,
        Completed,
        Failed
    };
    FName AssetName;      // Asset의 이름
    FName PackagePath;    // Asset의 패키지 경로
    EAssetType AssetType; // Asset의 타입
    uint32 Size;          // Asset의 크기 (바이트 단위)
    LoadState State;      // 로드 상태

    FString GetFullPath() const { return PackagePath.ToString() / AssetName.ToString(); }
};

struct FAssetRegistry
{
    TMap<FName, FAssetInfo> PathNameToAssetInfo;
};

class UAssetManager : public UObject
{
    DECLARE_CLASS(UAssetManager, UObject)

private:
    std::unique_ptr<FAssetRegistry> AssetRegistry;

public:
    UAssetManager();

    static bool IsInitialized();

    /** UAssetManager를 가져옵니다. */
    static UAssetManager& Get();

    /** UAssetManager가 존재하면 가져오고, 없으면 nullptr를 반환합니다. */
    static UAssetManager* GetIfInitialized();
    
    void InitAssetManager();

    const TMap<FName, FAssetInfo>& GetAssetRegistry();
    bool AddAsset(FString filePath);

    //void LoadAssetsOnScene();
    void LoadEntireAssets();

    void RegisterAsset(FString filePath, FAssetInfo::LoadState State);

    void RemoveAsset(FString filePath);

private:
    void OnLoaded(const FString& filename);

    void OnFailed(const FString& filename);

    bool AddAssetInternal(const FName& AssetName, const FAssetInfo& AssetInfo);

    bool SetAssetInternal(const FName& AssetName, const FAssetInfo& AssetInfo);

    bool ContainsInternal(const FName& AssetName);

    FSpinLock RegistryLock; // AssetRegistry에 접근할 때 쓰는 스핀락
};
