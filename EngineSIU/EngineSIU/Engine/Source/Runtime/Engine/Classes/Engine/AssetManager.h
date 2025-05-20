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

    ///** SavedParticleSystemMap에 안전하게 접근하기 위한 getter */
    //const TMap<FString, UParticleSystem*>& GetSavedParticleSystemMap() const
    //{
    //    return SavedParticleSystemMap;
    //}

    ///** 수정 가능하게도 필요하다면 non-const 버전 추가 */
    //TMap<FString, UParticleSystem*>& GetSavedParticleSystemMap()
    //{
    //    return SavedParticleSystemMap;
    //}
    TMap<FString, class UParticleSystem*> SavedParticleSystemMap;

private:
    void OnLoaded(const FString& filename);

    void OnFailed(const FString& filename);

    bool AddAssetInternal(const FName& AssetName, const FAssetInfo& AssetInfo);

    bool SetAssetInternal(const FName& AssetName, const FAssetInfo& AssetInfo);

    bool ContainsInternal(const FName& AssetName);

    FSpinLock RegistryLock; // AssetRegistry에 접근할 때 쓰는 스핀락

    // TMap<FString, class UParticleSystem*> SavedParticleSystemMap;
};
