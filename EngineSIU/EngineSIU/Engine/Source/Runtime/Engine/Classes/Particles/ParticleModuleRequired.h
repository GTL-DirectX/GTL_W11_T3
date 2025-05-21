#pragma once

#include "ParticleModule.h"

enum EParticleSortMode : int
{
    PSORTMODE_None,
    PSORTMODE_ViewProjDepth,
    PSORTMODE_DistanceToView,
    PSORTMODE_Age_OldestFirst,
    PSORTMODE_Age_NewestFirst,
    PSORTMODE_MAX,
};

/* 컷아웃 클리핑, SubUV, 모션 블러 처리에 활용되는 속성*/
struct FParticleRequiredModule
{
    uint32 NumFrames;             // 텍스처 시트를 여러 프레임으로 나눴을 때 총 프레임 수 
    uint32 NumBoundingVertices;   // 알파 컷아웃 마스크를 위한 버텍스 / 삼각형 수
    uint32 NumBoundingTriangles;
    float AlphaThreshold;         // 픽셀 알파 값이 이 값보다 작으면 클리핑 (렌더X)
    TArray<FVector2D> FrameData;  // 각 프레임에 대한 컷아웃 버텍스의 UV 좌표
    //FRHIShaderResourceView* BoundingGeometryBufferSRV;
    uint8 bCutoutTexureIsValid : 1;
    uint8 bUseVelocityForMotionBlur : 1;
};

/* 렌더링에 필요한 공통 파라미터(머티리얼, 색상, 정렬, Facing 등)을 정의
 * 모든 EmitterInstance를 상속받는 모든 클래스의 생성자에 인자로 들어감
 */

class UParticleModuleRequired : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleRequired, UParticleModule)

public:
    UParticleModuleRequired();
    virtual UObject* Duplicate(UObject* InOuter) override;

public:
    UMaterial* Material = nullptr;
    UPROPERTY(EditAnywhere, FVector, EmitterOrigin)
    UPROPERTY(EditAnywhere, FRotator, EmitterRotation)
    UPROPERTY(None, EParticleSortMode, SortMode)

    /* 	기본적으로 파티클은 컴포넌트 로컬 공간에서 시뮬레이션
     *  false면 캐릭터와 별개로 월드 공간에 떠다님
     */
    UPROPERTY(EditAnywhere, uint8, bUseLocalSpace)

    UPROPERTY(None, uint8, bKillOnDeactivate) // System이 비활성화 되면 emitter Kill

    UPROPERTY(None, uint8, bKillOnCompleted)  // Completed 시 emitter Kill

    // 초당 생성할 파티클 수
    // TODO : RawDistribution 으로 바꿔야 함
    float EmitterDuration;
    float SpawnRate;


    virtual void PostInitProperties() override;
    virtual void Spawn(FParticleEmitterInstance* Owner, int32 Offset, float SpawnTime, FBaseParticle* ParticleBase) override;
};
