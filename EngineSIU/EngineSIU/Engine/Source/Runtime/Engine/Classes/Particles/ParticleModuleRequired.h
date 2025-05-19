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


class UParticleModuleRequired : public UParticleModule
{
    DECLARE_CLASS(UParticleModuleRequired, UParticleModule)

public:
    UParticleModuleRequired() = default;

    // class UMaterialInterface* Material;
    
    UPROPERTY(EditAnywhere, FVector, EmitterOrigin)
    
    UPROPERTY(EditAnywhere, FRotator, EmitterRotation)

    UPROPERTY(None, EParticleSortMode, SortMode)

    UPROPERTY(EditAnywhere, uint8, bUseLocalSpace)

    UPROPERTY(None, uint8, bKillOnDeactivate) // System이 비활성화 되면 emitter Kill

    UPROPERTY(None, uint8, bKillOnCompleted)  // Completed 시 emitter Kill
};
