#pragma once

#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"
#include "Math/Color.h"
#include "Container/Array.h"
#include "DirectXTK/BufferHelpers.h"
#include "Components/Material/Material.h"

class UMaterial;
/*-----------------------------------------------------------------------------
    Forward declarations
-----------------------------------------------------------------------------*/
//	Emitter and module types
class UParticleEmitter;
class UParticleSpriteEmitter;
class UParticleModule;

class UStaticMesh;
class UStaticMeshComponent;

class UParticleSystem;
class UParticleSystemComponent;

class UParticleLODLevel;

class USkeletalMeshComponent;

struct FParticleSpriteEmitterInstance;
struct FParticleMeshEmitterInstance;
struct FParticleBeam2EmitterInstance;

/*-----------------------------------------------------------------------------
    Helper macros.
-----------------------------------------------------------------------------*/

#define DECLARE_PARTICLE(Name,Address)		\
	FBaseParticle& Name = *((FBaseParticle*) (Address));

#define DECLARE_PARTICLE_CONST(Name,Address)		\
	const FBaseParticle& Name = *((const FBaseParticle*) (Address));

#define DECLARE_PARTICLE_PTR(Name,Address)		\
	FBaseParticle* Name = (FBaseParticle*) (Address);

/* 원본 : check((Owner != NULL) && (Owner->Component != NULL)); */
#define BEGIN_UPDATE_LOOP																								\
	{																													\
		static_assert((Component != NULL));													\
		int32&			ActiveParticles = Owner->ActiveParticles;														\
		uint32			CurrentOffset	= Offset;																		\
		const uint8*		ParticleData	= Owner->ParticleData;															\
		const uint32		ParticleStride	= Owner->ParticleStride;														\
		uint16*			ParticleIndices	= Owner->ParticleIndices;														\
		for(int32 i=ActiveParticles-1; i>=0; i--)																			\
		{																												\
			const int32	CurrentIndex	= ParticleIndices[i];															\
			const uint8* ParticleBase	= ParticleData + CurrentIndex * ParticleStride;									\
			FBaseParticle& Particle		= *((FBaseParticle*) ParticleBase);												\
			if (/*(Particle.Flags & STATE_Particle_Freeze) == 0*/ true)															\
			{																											\

#define END_UPDATE_LOOP																									\
			}																											\
			CurrentOffset				= Offset;																		\
		}																												\
	}

#define CONTINUE_UPDATE_LOOP																							\
		CurrentOffset = Offset;																							\
		continue;

#define SPAWN_INIT																										\
	if (Owner == NULL && Owner->Component == NULL) \
    { \
        return; \
    }								\
	const int32		ActiveParticles	= Owner->ActiveParticles;															\
	const uint32		ParticleStride	= Owner->ParticleStride;															\
	uint32			CurrentOffset	= Offset;																			\
	FBaseParticle&	Particle		= *(ParticleBase);

#define PARTICLE_ELEMENT(Type,Name)																						\
	Type& Name = *((Type*)((uint8*)ParticleBase + CurrentOffset));																\
	CurrentOffset += sizeof(Type);

#define KILL_CURRENT_PARTICLE																							\
	{																													\
		ParticleIndices[i]					= ParticleIndices[ActiveParticles-1];										\
		ParticleIndices[ActiveParticles-1]	= CurrentIndex;																\
		ActiveParticles--;																								\
	}

#define BEGIN_MY_UPDATE_LOOP \
    for (int32 i = ActiveParticles - 1; i >= 0; i--) \
    { \
        const int32 ParticleIndex = ParticleIndices[i]; \
        uint8* ParticleBase = ParticleData + ParticleIndex * ParticleStride; \
        FBaseParticle& Particle = *(FBaseParticle*)ParticleBase;

#define END_MY_UPDATE_LOOP \
    }

/*
    파티클 시스템이 실제로 시뮬레이션하고 렌더링하는 **파티클 한 개의 데이터**를 저장하는 구조체.
    SIMD 정렬을 위해 바이트 정렬이 적용.
    **UE와 데이터 차이가 있기에 확인 필요.
*/
struct FBaseParticle
{
    // 24 bytes
    FVector		OldLocation;			
    FVector		Location;				

    // 16 bytes
    FVector		BaseVelocity;			
    float			Rotation;				

    // 16 bytes
    FVector 		Velocity;				
    float			BaseRotationRate;		

    // 16 bytes
    FVector	    	BaseSize;				
    float			RotationRate;			

    // 16 bytes
    FVector 		Size;					
    int32			Flags;					

    // 16 bytes
    FLinearColor	Color;					

    // 16 bytes
    FLinearColor	BaseColor;

    // 16 bytes
    float			RelativeTime;			// Relative time, range is 0 (==spawn) to 1 (==death). RelativeTime >= 1.0f 이면 Kill. RelativeTime += DeltaTime / LifeTime.
    float			OneOverMaxLifetime;		// 1초당 늘어날 수명 = 1/MaxLifeTime [0, 1]
    float			Placeholder0;
    float			Placeholder1;
};

enum EParticleStates
{
    /** Ignore updates to the particle						*/
    STATE_Particle_JustSpawned = 0x02000000,
    /** Ignore updates to the particle						*/
    STATE_Particle_Freeze = 0x04000000,
    /** Ignore collision updates to the particle			*/
    STATE_Particle_IgnoreCollisions = 0x08000000,
    /**	Stop translations of the particle					*/
    STATE_Particle_FreezeTranslation = 0x10000000,
    /**	Stop rotations of the particle						*/
    STATE_Particle_FreezeRotation = 0x20000000,
    /** Combination for a single check of 'ignore' flags	*/
    STATE_Particle_CollisionIgnoreCheck = STATE_Particle_Freeze | STATE_Particle_IgnoreCollisions | STATE_Particle_FreezeTranslation | STATE_Particle_FreezeRotation,
    /** Delay collision updates to the particle				*/
    STATE_Particle_DelayCollisions = 0x40000000,
    /** Flag indicating the particle has had at least one collision	*/
    STATE_Particle_CollisionHasOccurred = 0x80000000,
    /** State mask. */
    STATE_Mask = 0xFE000000,
    /** Counter mask. */
    STATE_CounterMask = (~STATE_Mask)
};

/**
 * Per-particle data sent to the GPU.
 */
struct FParticleSpriteVertex
{
    FVector Position;
    float RelativeTime;
    FVector	OldPosition;
    float ParticleId;
    FVector2D Size;
    float Rotation;
    float SubImageIndex;
    FLinearColor Color;

    inline const static D3D11_INPUT_ELEMENT_DESC LayoutDesc[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TEXCOORD", 0, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},       // RelativeTime
        {"TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0}, // OldPosition
        {"TEXCOORD", 2, DXGI_FORMAT_R32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0},       // ParticleId
        {"TEXCOORD", 3, DXGI_FORMAT_R32G32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},    // Size
        {"TEXCOORD", 4, DXGI_FORMAT_R32_FLOAT, 0, 40, D3D11_INPUT_PER_VERTEX_DATA, 0},       // Rotation
        {"TEXCOORD", 5, DXGI_FORMAT_R32_FLOAT, 0, 44, D3D11_INPUT_PER_VERTEX_DATA, 0},       // SubImageIndex
        {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D11_INPUT_PER_VERTEX_DATA, 0}, // Color
    };
};

/**
 * Per-particle data sent to the GPU.
 */
struct FParticleSpriteVertexNonInstanced
{
    FVector2D UV;
    FVector Position;
    float RelativeTime;
    FVector	OldPosition;
    float ParticleId;
    FVector2D Size;
    float Rotation;
    float SubImageIndex;
    FLinearColor Color;
};

//	FParticleSpriteVertexDynamicParameter
struct FParticleVertexDynamicParameter
{
    /** The dynamic parameter of the particle			*/
    // 슬롯이 4개인 이유는 UE Material의 Dynamic Parameter가 최대 4개의 슬롯을 지원하기 때문.
    float			DynamicValue[4]; // x, y, z, w
};
#include <cstddef> // offsetof
struct FMeshParticleInstanceVertex
{
    FLinearColor   Color;           // 16 bytes
    FMatrix       Transform;     // 64 bytes (4×4 행렬)
    FVector4       Velocity;        // 16 bytes
    //int16_t        SubUVParams[4];  // 8 bytes
    //float          SubUVLerp;       // 4 bytes
    //float          RelativeTime;    // 4 bytes
    // → 총합 112 bytes
    inline const static D3D11_INPUT_ELEMENT_DESC LayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,   0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0,  12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0,  24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
};

struct FMeshParticleInstanceVertexDynamicParameter
{
    /** The dynamic parameter of the particle. */
    float DynamicValue[4];
};

struct FMeshParticleInstanceVertexPrevTransform
{
    FVector4 PrevTransform0;
    FVector4 PrevTransform1;
    FVector4 PrevTransform2;
};

//
//	SubUV-related payloads
//
struct FFullSubUVPayload
{
    // The integer portion indicates the sub-image index.
    // The fractional portion indicates the lerp factor.
    float ImageIndex;
    float RandomImageTime;
};

//
//	AttractorParticle
//
struct FAttractorParticlePayload
{
    int32			SourceIndex;
    uint32		SourcePointer;
    FVector		SourceVelocity;
};

struct FLightParticlePayload
{
    FVector		ColorScale;
    uint64		LightId;
    float		RadiusScale;
    float		LightExponent;
    float		InverseExposureBlend;
    bool		bValid;
    bool		bAffectsTranslucency;
    bool		bHighQuality;
};

// 시작점(Start)과 종료점(End)을 갖는 선형 연결 파티클.
// 레이저, 번개, 에너지빔, 갈고리 등의 선처럼 뻗는 이펙트에 사용.
struct FBeam2TypeDataPayload
{
    FVector		SourcePoint;
    FVector		SourceTangent;
    float		SourceStrength;

    FVector		TargetPoint;
    FVector		TargetTangent;
    float		TargetStrength;

    int32		Lock_Max_NumNoisePoints;
    int32		InterpolationSteps;

    FVector		Direction;
    double		StepSize;
    int32		Steps;
    float		TravelRatio;

    int32		TriangleCount;

    /**
     *	Type and indexing flags
     * 3               1              0
     * 1...|...|...|...5...|...|...|..0
     * TtPppppppppppppppNnnnnnnnnnnnnnn
     * Tt				= Type flags --> 00 = Middle of Beam (nothing...)
     * 									 01 = Start of Beam
     * 									 10 = End of Beam
     * Ppppppppppppppp	= Previous index
     * Nnnnnnnnnnnnnnn	= Next index
     * 		int32				Flags;
     *
     * NOTE: These values DO NOT get packed into the vertex buffer!
     */
    int32			Flags;
};

/**	Particle Source/Target Data Payload									*/
struct FBeamParticleSourceTargetPayloadData
{
    int32			ParticleIndex;
};

/**	Particle Source Branch Payload										*/
struct FBeamParticleSourceBranchPayloadData
{
    int32			NoiseIndex;
};


/** Particle Beam Modifier Data Payload */
struct FBeamParticleModifierPayloadData
{
    uint32	bModifyPosition : 1;
    uint32	bScalePosition : 1;
    uint32	bModifyTangent : 1;
    uint32	bScaleTangent : 1;
    uint32	bModifyStrength : 1;
    uint32	bScaleStrength : 1;
    FVector	Position;
    FVector	Tangent;
    float	Strength;

    // Helper functions
    FORCEINLINE void UpdatePosition(FVector& Value)
    {
        if (bModifyPosition == true)
        {
            if (bScalePosition == false)
            {
                Value += Position;
            }
            else
            {
                Value *= Position;
            }
        }
    }

    //FORCEINLINE void UpdateTangent(FVector& Value, bool bAbsolute)
    //{
    //    if (bModifyTangent == true)
    //    {
    //        FVector ModTangent;
    //        if (bAbsolute == false)
    //        {
    //            // Transform the modified tangent so it is relative to the real tangent
    //            const FQuat RotQuat = FQuat::FindBetweenNormals(FVector(1.0f, 0.0f, 0.0f), Value);
    //            ModTangent = RotQuat.RotateVector(Tangent);
    //        }
    //        else
    //        {
    //            ModTangent = Tangent;
    //        }

    //        if (bScaleTangent == false)
    //        {
    //            Value += ModTangent;
    //        }
    //        else
    //        {
    //            Value *= ModTangent;
    //        }
    //    }
    //}

    FORCEINLINE void UpdateStrength(float& Value)
    {
        if (bModifyStrength == true)
        {
            if (bScaleStrength == false)
            {
                Value += Strength;
            }
            else
            {
                Value *= Strength;
            }
        }
    }
};


/** Trails Base data payload */
// 시간 흐름에 따라 움직임을 따라가는 잔상/궤적 효과 표현 파티클.
struct FTrailsBaseTypeDataPayload
{
    /**
     * TRAIL_EMITTER_FLAG_MASK         0xf0000000
     * TRAIL_EMITTER_PREV_MASK         0x0fffc000
     * TRAIL_EMITTER_PREV_SHIFT        14
     * TRAIL_EMITTER_NEXT_MASK         0x00003fff
     * TRAIL_EMITTER_NEXT_SHIFT        0

     *	Type and indexing flags
     *	3               1              0
     *	1...|...|...|...5...|...|...|..0
     *	TtttPpppppppppppppNnnnnnnnnnnnnn
     *
     *	Tttt = Type flags
     *		0x0 = ForceKill	- the trail should be completely killed in the next KillParticles call.
     *		0x1	= DeadTrail	- the trail should no longer spawn particles. Just let it die out as particles in it fade.
     *		0x2	= Middle	- indicates this is a particle in the middle of a trail.
     *		0x4	= Start		- indicates this is the first particle in a trail.
     *		0x8	= End		- indicates this is the last particle in a trail.
     *	Pppppppppppppp	= Previous index
     *	Nnnnnnnnnnnnnn	= Next index
     */
    int32 Flags;
    /** The trail index - valid in a START particle only */
    int32 TrailIndex;
    /** The number of triangles in the trail - valid in a START particle only */
    int32 TriangleCount;
    /** The time that the particle was spawned */
    float SpawnTime;
    /** The time slice when the particle was spawned */
    float SpawnDelta;
    /** The starting tiled U value for this particle */
    float TiledU;
    /** The tessellated spawn points between this particle and the next one */
    int32 SpawnedTessellationPoints;
    /** The number of points to interpolate between this particle and the next when rendering */
    int32 RenderingInterpCount;
    /** The scale factor to use to shrink up in tight curves */
    float PinchScaleFactor;
    /** true if the particle is an interpolated spawn, false if true position based. */
    uint32 bInterpolatedSpawn : 1;
    /** true if the particle was spawn via movement, false if not. */
    uint32 bMovementSpawned : 1;
};

struct FRibbonTypeDataPayload : public FTrailsBaseTypeDataPayload
{
    /**	Tangent for the trail segment */
    FVector Tangent;
    /**	The 'up' for the segment (render plane) */
    FVector Up;
    /** The source index tracker (particle index, etc.) */
    int32 SourceIndex;
};

/** AnimTrail payload */
struct FAnimTrailTypeDataPayload : public FTrailsBaseTypeDataPayload
{
    //Direction from the first socket sample to the second.
    FVector Direction;
    //Tangent of the curve.
    FVector Tangent;
    //Half length between the sockets. First vertex = Location - Dir * Length; Second vertex = Location + Dir * Lenght
    float Length;
    /** Parameter of this knot on the spline*/
    float InterpolationParameter;
};

/** Mesh rotation data payload										*/
struct FMeshRotationPayloadData
{
    FVector	 InitialOrientation;		// from mesh data module
    FVector  InitRotation;				// from init rotation module
    FVector  Rotation;
    FVector	 CurContinuousRotation;
    FVector  RotationRate;
    FVector  RotationRateBase;
};

struct FMeshMotionBlurPayloadData
{
    FVector BaseParticlePrevVelocity;
    FVector BaseParticlePrevSize;
    FVector PayloadPrevRotation;
    FVector PayloadPrevOrbitOffset;
    float   BaseParticlePrevRotation;
    float   PayloadPrevCameraOffset;
};

/** ModuleLocationEmitter instance payload							*/
struct FLocationEmitterInstancePayload
{
    int32		LastSelectedIndex;
};

/** Random-seed instance payload */
struct FParticleRandomSeedInstancePayload
{
    //FRandomStream	RandomStream; -> Random 필요할 때 생성해서 사용하면 됨.
};


/*-----------------------------------------------------------------------------
    Particle Sorting Helper
-----------------------------------------------------------------------------*/
struct FParticleOrder
{
    int32 ParticleIndex;

    union
    {
        float Z;
        uint32 C;
    };

    FParticleOrder(int32 InParticleIndex, float InZ) :
        ParticleIndex(InParticleIndex),
        Z(InZ)
    {
    }

    FParticleOrder(int32 InParticleIndex, uint32 InC) :
        ParticleIndex(InParticleIndex),
        C(InC)
    {
    }
};


/*-----------------------------------------------------------------------------
    Async Fill Organizational Structure
-----------------------------------------------------------------------------*/
// 파티클 시스템의 Vertex/Index 버퍼를 비동기적으로 채우기 위한 구조체.
struct FAsyncBufferFillData
{
    /** Local to world transform. */
    FMatrix LocalToWorld;
    /** World to local transform. */
    FMatrix WorldToLocal;
    /** View for this buffer fill task   */
    //const FSceneView* View; // 우린 안쓰는 데이터.
    /** Number of verts in VertexData   */
    int32									VertexCount;
    /** Stride of verts, used only for error checking   */
    int32									VertexSize;
    /** Pointer to vertex data   */
    void* VertexData;
    /** Number of indices in IndexData   */
    int32									IndexCount;
    /** Pointer to index data   */
    void* IndexData;
    /** Number of triangles filled in   */
    int32									OutTriangleCount;
    /** Pointer to dynamic parameter data */
    void* DynamicParameterData;

    /** Constructor, just zeros everything   */
    FAsyncBufferFillData()
    {
        // this is all POD
        FPlatformMemory::MemZero(this, sizeof(FAsyncBufferFillData));
    }
    /** Destructor, frees memory and zeros everything   */
    ~FAsyncBufferFillData()
    {
        FPlatformMemory::MemZero(this, sizeof(FAsyncBufferFillData));
    }
};

enum EDynamicEmitterType
{
    DET_Unknown = 0,
    DET_Sprite,
    DET_Mesh,
    DET_Beam2,
    DET_Ribbon,
    DET_AnimTrail,
    DET_Custom
};

struct FParticleDataContainer
{
    int32 MemBlockSize;
    int32 ParticleDataNumBytes;
    int32 ParticleIndicesNumShorts;
    uint8* ParticleData; // this is also the memory block we allocated
    uint16* ParticleIndices; // not allocated, this is at the end of the memory block

    FParticleDataContainer()
        : MemBlockSize(0)
        , ParticleDataNumBytes(0)
        , ParticleIndicesNumShorts(0)
        , ParticleData(nullptr)
        , ParticleIndices(nullptr)
    {
    }
    ~FParticleDataContainer()
    {
        Free();
    }
    void Alloc(int32 InParticleDataNumBytes, int32 InParticleIndicesNumShorts);
    void Free();
};

/** Source data base class for all emitter types */
struct FDynamicEmitterReplayDataBase
{
    EDynamicEmitterType	eEmitterType;
    int32 ActiveParticleCount;
    int32 ParticleStride;
    FParticleDataContainer DataContainer;

    FVector Scale;
    int32 SortMode;

    FDynamicEmitterReplayDataBase()
        : eEmitterType(DET_Unknown),
        ActiveParticleCount(0),
        ParticleStride(0),
        Scale(FVector(1.0f)),
        SortMode(0)	// Default to PSORTMODE_None
    {
    }

    virtual ~FDynamicEmitterReplayDataBase()
    {
    }

    virtual void Serialize(FArchive& Ar)
    {
        int32 EmitterTypeAsInt = eEmitterType;
        Ar << EmitterTypeAsInt;
        eEmitterType = static_cast<EDynamicEmitterType>(EmitterTypeAsInt);

        Ar << ActiveParticleCount;
        Ar << ParticleStride;

        TArray<uint8> ParticleData;
        TArray<uint16> ParticleIndices;

        if (!Ar.IsLoading() /*&& !Ar.IsObjectReferenceCollector()*/)
        {
            if (DataContainer.ParticleDataNumBytes)
            {
                ParticleData.AddUninitialized(DataContainer.ParticleDataNumBytes);
                FPlatformMemory::Memcpy(ParticleData.GetData(), DataContainer.ParticleData, DataContainer.ParticleDataNumBytes);
            }
            if (DataContainer.ParticleIndicesNumShorts)
            {
                ParticleIndices.AddUninitialized(DataContainer.ParticleIndicesNumShorts);
                FPlatformMemory::Memcpy(ParticleIndices.GetData(), DataContainer.ParticleIndices, DataContainer.ParticleIndicesNumShorts * sizeof(uint16));
            }
        }

        Ar << ParticleData;
        Ar << ParticleIndices;

        if (Ar.IsLoading())
        {
            DataContainer.Free();
            if (ParticleData.Num())
            {
                DataContainer.Alloc(ParticleData.Num(), ParticleIndices.Num());
                FPlatformMemory::Memcpy(DataContainer.ParticleData, ParticleData.GetData(), DataContainer.ParticleDataNumBytes);
                if (DataContainer.ParticleIndicesNumShorts)
                {
                    FPlatformMemory::Memcpy(DataContainer.ParticleIndices, ParticleIndices.GetData(), DataContainer.ParticleIndicesNumShorts * sizeof(uint16));
                }
            }
            else
            {
                assert(!ParticleIndices.Num());
            }
        }

        Ar << Scale;
        Ar << SortMode;
    }
};

/** Source data base class for Sprite emitters */
struct FDynamicSpriteEmitterReplayDataBase
    : public FDynamicEmitterReplayDataBase
{
    UMaterial* Material;
    struct FParticleRequiredModule* RequiredModule;
    FVector							NormalsSphereCenter;
    FVector							NormalsCylinderDirection;
    float							InvDeltaSeconds;
    FVector						    LWCTile;
    int32							MaxDrawCount;
    int32							OrbitModuleOffset;
    int32							DynamicParameterDataOffset;
    int32							LightDataOffset;
    float							LightVolumetricScatteringIntensity;
    int32							CameraPayloadOffset;
    int32							SubUVDataOffset;
    int32							SubImages_Horizontal;
    int32							SubImages_Vertical;
    bool						    bUseLocalSpace;
    bool						    bLockAxis;
    uint8						    ScreenAlignment;
    uint8						    LockAxisFlag;
    uint8						    EmitterRenderMode;
    uint8						    EmitterNormalsMode;
    FVector2D					    PivotOffset;
    bool						    bUseVelocityForMotionBlur;
    bool						    bRemoveHMDRoll;
    float						    MinFacingCameraBlendDistance;
    float						    MaxFacingCameraBlendDistance;

    /** Constructor */
    FDynamicSpriteEmitterReplayDataBase();
    ~FDynamicSpriteEmitterReplayDataBase();

    /** Serialization */
    virtual void Serialize(FArchive& Ar);

};

struct FDynamicMeshEmitterReplayData
    : public FDynamicSpriteEmitterReplayDataBase
{
    int32	SubUVInterpMethod;
    int32	SubUVDataOffset;
    int32	SubImages_Horizontal;
    int32	SubImages_Vertical;
    bool	bScaleUV;
    int32	MeshRotationOffset;
    int32	MeshMotionBlurOffset;
    uint8	MeshAlignment;
    bool	bMeshRotationActive;
    FVector	LockedAxis;

    FDynamicMeshEmitterReplayData() :
        SubUVInterpMethod(0),
        SubUVDataOffset(0),
        SubImages_Horizontal(0),
        SubImages_Vertical(0),
        bScaleUV(false),
        MeshRotationOffset(0),
        MeshMotionBlurOffset(0),
        MeshAlignment(0),
        bMeshRotationActive(false),
        LockedAxis(1.0f, 0.0f, 0.0f)
    {
    }


    virtual void Serialize(FArchive& Ar)
    {
        FDynamicSpriteEmitterReplayDataBase::Serialize(Ar);

        Ar << SubUVInterpMethod;
        Ar << SubUVDataOffset;
        Ar << SubImages_Horizontal;
        Ar << SubImages_Vertical;
        Ar << bScaleUV;
        Ar << MeshRotationOffset;
        Ar << MeshMotionBlurOffset;
        Ar << MeshAlignment;
        Ar << bMeshRotationActive;
        Ar << LockedAxis;
    }

};


/** Base class for all emitter types */
// 주석된 부분은 언리얼에서 사용되는 형태.
// 현재 엔진에서는 FSceneView, SceneProxy를 사용하지 않기에 우회 방법 필요. or FSceneView, SceneProxy 구현.
struct FDynamicEmitterDataBase
{
    FDynamicEmitterDataBase(const class UParticleModuleRequired* RequiredModule);

    virtual ~FDynamicEmitterDataBase()
    {
    }

    virtual const FDynamicEmitterReplayDataBase& GetSource() const = 0;

    uint32	bSelected : 1;
    uint32	bValid : 1;

    int32  EmitterIndex;
};

struct FDynamicSpriteEmitterDataBase : public FDynamicEmitterDataBase
{
    FDynamicSpriteEmitterDataBase(const UParticleModuleRequired* RequiredModule) :
        FDynamicEmitterDataBase(RequiredModule),
        bUsesDynamicParameter(false)
    {
    }

    virtual ~FDynamicSpriteEmitterDataBase()
    {
    }

    void SortSpriteParticles(int32 SortMode, bool bLocalSpace,
        int32 ParticleCount, const uint8* ParticleData, int32 ParticleStride, const uint16* ParticleIndices,
        /*const FSceneView* View,*/ const FMatrix& LocalToWorld, FParticleOrder* ParticleOrder) const;


    virtual int32 GetDynamicVertexStride(/*ERHIFeatureLevel::Type *//*InFeatureLevel*/) const
    {
        //checkf(0, TEXT("GetDynamicVertexStride MUST be overridden"));
        return 0;
    }

    virtual int32 GetDynamicParameterVertexStride() const
    {
        assert(0, TEXT("GetDynamicParameterVertexStride MUST be overridden"));
        return 0;
    }

    virtual const FDynamicSpriteEmitterReplayDataBase* GetSourceData() const
    {
        assert(0, TEXT("GetSourceData MUST be overridden"));
        return NULL;
    }

    virtual void GetIndexAllocInfo(int32& OutNumIndices, int32& OutStride) const
    {
        assert(0, TEXT("GetIndexAllocInfo is not valid for this class."));
    }

    //virtual void RenderDebug(const FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, bool bCrosses) const;

    virtual void DoBufferFill(FAsyncBufferFillData& Me) const
    {
        // Must be overridden if called
        assert(0);
    }

    uint32 bUsesDynamicParameter : 1;
};


struct FDynamicSpriteEmitterData : public FDynamicSpriteEmitterDataBase
{
public:

    FDynamicSpriteEmitterData(const UParticleModuleRequired* RequiredModule) :
        FDynamicSpriteEmitterDataBase(RequiredModule)
    {
    }
    ~FDynamicSpriteEmitterData() {}

    virtual void Init(bool bInSelected);
    
    virtual int32 GetDynamicVertexStride(/*ERHIFeatureLevel::Type InFeatureLevel*/) const override // ERHIFeatureLevel::Type 은 렌더 플랫폼에 관련된 정보이므로 필요 없음.
    {
        return sizeof(FParticleSpriteVertex);
    }

    virtual int32 GetDynamicParameterVertexStride() const override
    {
        return sizeof(FParticleVertexDynamicParameter);
    }

    virtual const FDynamicSpriteEmitterReplayDataBase* GetSourceData() const override
    {
        return &Source;
    }

    virtual const FDynamicEmitterReplayDataBase& GetSource() const override
    {
        return Source;
    }

    virtual bool GetVertexAndIndexData(void* VertexData, void* DynamicParameterVertexData, void* FillIndexData, FParticleOrder* ParticleOrder, const FVector& InCameraPosition, const FMatrix& InLocalToWorld, uint32 InstanceFactor) const;

    virtual bool GetVertexAndIndexDataNonInstanced(void* VertexData, void* DynamicParameterVertexData, void* FillIndexData, FParticleOrder* ParticleOrder, const FVector& InCameraPosition, const FMatrix& InLocalToWorld, int32 NumVerticesPerParticle) const;

    ID3D11Buffer* VertexBuffer       = nullptr;  // 정점 버퍼 (퍼-프레임 동적 업데이트)
    ID3D11Buffer* IndexBuffer        = nullptr;  // 인덱스 버퍼
    ID3D11Buffer* DynamicParamBuffer = nullptr;  // (옵션) 다이나믹 파라미터용 상수 버퍼

    FDynamicSpriteEmitterReplayDataBase Source;
};


/** Dynamic emitter data for Mesh emitters
 * SpriteEmitterDataBase를 상속받는 이유 : 기본 Sprite인 상태에서 Mesh 추가하는 듯
 */
struct FDynamicMeshEmitterData : public FDynamicSpriteEmitterData /* 야매 : FDynamicSpriteEmitterDataBase*/
{
    FDynamicMeshEmitterData(const UParticleModuleRequired* RequiredModule);

    virtual void Init(bool bInSelected) override;

    void CalculateParticleTransform(
        const FMatrix& ProxyLocalToWorld,
        const FVector& ParticleLocation,
        float    ParticleRotation,
        const FVector& ParticleVelocity,
        const FVector& ParticleSize,
        const FVector& ParticlePayloadInitialOrientation,
        const FVector& ParticlePayloadRotation,
        const FVector& ParticlePayloadCameraOffset,
        const FVector& ParticlePayloadOrbitOffset,
        const FVector& ViewOrigin,
        const FVector& ViewDirection,
        FMatrix& OutTransformMat
    ) const;

    virtual bool GetVertexAndIndexData(void* VertexData, void* DynamicParameterVertexData, void* FillIndexData, FParticleOrder* ParticleOrder, const FVector& InCameraPosition, const FMatrix& InLocalToWorld, uint32 InstanceFactor) const;

    virtual int32 GetDynamicVertexStride(/*ERHIFeatureLevel::Type*/ /*InFeatureLevel*/) const override
    {
        return sizeof(FMeshParticleInstanceVertex);
    }

    virtual int32 GetDynamicParameterVertexStride() const override
    {
        return sizeof(FMeshParticleInstanceVertexDynamicParameter);
    }

    virtual const FDynamicSpriteEmitterReplayDataBase* GetSourceData() const override
    {
        return &Source;
    }

    /** Returns the source data for this particle system */
    virtual const FDynamicEmitterReplayDataBase& GetSource() const override
    {
        return Source;
    }

    UStaticMesh* StaticMesh;

    FDynamicMeshEmitterReplayData SourceMesh; // 야매코드 : Source여야 함

    int32 LastFramePreRendered;


    uint32 MeshTypeDataOffset;

    uint32 bApplyPreRotation : 1;
    uint32 bUseMeshLockedAxis : 1;
    uint32 bUseCameraFacing : 1;
    uint32 bApplyParticleRotationAsSpin : 1;
    uint32 bFaceCameraDirectionRatherThanPosition : 1;
    uint8 CameraFacingOption;

    bool bUseStaticMeshLODs;
    float LODSizeScale;
    mutable int32 LastCalculatedMeshLOD;
    const FParticleMeshEmitterInstance* EmitterInstance;
};
