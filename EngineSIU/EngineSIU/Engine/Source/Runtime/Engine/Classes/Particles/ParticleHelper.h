#pragma once

#include "Math/Vector.h"
#include "Math/Vector4.h"
#include "Math/Matrix.h"
#include "Math/Color.h"
#include "Container/Array.h"

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

#define BEGIN_UPDATE_LOOP																								\
	{																													\
		check((Owner != NULL) && (Owner->Component != NULL));															\
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
			if ((Particle.Flags & STATE_Particle_Freeze) == 0)															\
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
	check((Owner != NULL) && (Owner->Component != NULL));																\
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


/*
    파티클 시스템이 실제로 시뮬레이션하고 렌더링하는 **파티클 한 개의 데이터**를 저장하는 구조체.
    SIMD 정렬을 위해 바이트 정렬이 적용.
    **UE와 데이터 차이가 있기에 확인 필요.
*/
struct FBaseParticle
{
    // 24 bytes
    FVector		OldLocation;			// Last frame's location, used for collision
    FVector		Location;				// Current location. Loaction += Velocity * DeltaTime.

    // 16 bytes
    FVector		BaseVelocity;			// Velocity = BaseVelocity at the start of each frame.
    float			Rotation;				// Rotation of particle (in Radians)

    // 16 bytes
    FVector 		Velocity;				// Current velocity, gets reset to BaseVelocity each frame to allow 
    float			BaseRotationRate;		// Initial angular velocity of particle (in Radians per second)

    // 16 bytes
    FVector	    	BaseSize;				// Size = BaseSize at the start of each frame
    float			RotationRate;			// Current rotation rate, gets reset to BaseRotationRate each frame

    // 16 bytes
    FVector 		Size;					// Current size, gets reset to BaseSize each frame
    int32			Flags;					// Flags indicating various particle states

    // 16 bytes
    FLinearColor	Color;					// Current color of particle.

    // 16 bytes
    FLinearColor	BaseColor;				// Base color of the particle

    // 16 bytes
    float			RelativeTime;			// Relative time, range is 0 (==spawn) to 1 (==death). RelativeTime >= 1.0f 이면 Kill. RelativeTime += DeltaTime / LifeTime.
    float			OneOverMaxLifetime;		// Reciprocal of lifetime. 1 / Lifetime 계산을 매번 하지 않기 위한 캐시 값.
    float			Placeholder0;
    float			Placeholder1;
};

/**
 * Per-particle data sent to the GPU.
 */
struct FParticleSpriteVertex
{
    /** The position of the particle. */
    FVector Position;
    /** The relative time of the particle. */
    float RelativeTime;
    /** The previous position of the particle. */
    FVector	OldPosition;
    /** Value that remains constant over the lifetime of a particle. */
    float ParticleId;
    /** The size of the particle. */
    FVector2D Size;
    /** The rotation of the particle. */
    float Rotation;
    /** The sub-image index for the particle. */
    float SubImageIndex;
    /** The color of the particle. */
    FLinearColor Color;
};

/**
 * Per-particle data sent to the GPU.
 */
struct FParticleSpriteVertexNonInstanced
{
    /** The texture UVs. */
    FVector2D UV;
    /** The position of the particle. */
    FVector Position;
    /** The relative time of the particle. */
    float RelativeTime;
    /** The previous position of the particle. */
    FVector	OldPosition;
    /** Value that remains constant over the lifetime of a particle. */
    float ParticleId;
    /** The size of the particle. */
    FVector2D Size;
    /** The rotation of the particle. */
    float Rotation;
    /** The sub-image index for the particle. */
    float SubImageIndex;
    /** The color of the particle. */
    FLinearColor Color;
};

//	FParticleSpriteVertexDynamicParameter
struct FParticleVertexDynamicParameter
{
    /** The dynamic parameter of the particle			*/
    // 슬롯이 4개인 이유는 UE Material의 Dynamic Parameter가 최대 4개의 슬롯을 지원하기 때문.
    float			DynamicValue[4]; // x, y, z, w
};

// Per-particle data sent to the GPU.
struct FMeshParticleInstanceVertex
{
    /** The color of the particle. */
    FLinearColor Color;

    /** The instance to world transform of the particle. Translation vector is packed into W components. */
    FVector4 Transform[3];

    /** The velocity of the particle, XYZ: direction, W: speed. */
    FVector4 Velocity;

    /** The sub-image texture offsets for the particle. */
    int16 SubUVParams[4];

    /** The sub-image lerp value for the particle. */
    float SubUVLerp;

    /** The relative time of the particle. */
    float RelativeTime;
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
    /** The source of this beam											*/
    FVector		SourcePoint;
    /** The source tangent of this beam									*/
    FVector		SourceTangent;
    /** The stength of the source tangent of this beam					*/
    float		SourceStrength;

    /** The target of this beam											*/
    FVector		TargetPoint;
    /** The target tangent of this beam									*/
    FVector		TargetTangent;
    /** The stength of the Target tangent of this beam					*/
    float		TargetStrength;

    /** Target lock, extreme max, Number of noise points				*/
    int32		Lock_Max_NumNoisePoints;

    /** Number of segments to render (steps)							*/
    int32		InterpolationSteps;

    /** Direction to step in											*/
    FVector		Direction;
    /** StepSize (for each segment to be rendered)						*/
    double		StepSize;
    /** Number of segments to render (steps)							*/
    int32		Steps;
    /** The 'extra' amount to travel (partial segment)					*/
    float		TravelRatio;

    /** The number of triangles to render for this beam					*/
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
    /**	The type of emitter. */
    EDynamicEmitterType	eEmitterType;

    /**	The number of particles currently active in this emitter. */
    int32 ActiveParticleCount;

    int32 ParticleStride;
    FParticleDataContainer DataContainer;

    FVector Scale;

    /** Whether this emitter requires sorting as specified by artist.	*/
    int32 SortMode;

    /** Constructor */
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

    /** Serialization */
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
    //UMaterialInterface* MaterialInterface; // TODO: 머티리얼 관련 부분 Interface 사용하도록 바꿔줘야 할 듯. 아니면 Material 객체로 대체.
    struct FParticleRequiredModule* RequiredModule;
    FVector							NormalsSphereCenter;
    FVector							NormalsCylinderDirection;
    float							InvDeltaSeconds;
    FVector						LWCTile;
    int32							MaxDrawCount;
    int32							OrbitModuleOffset;
    int32							DynamicParameterDataOffset;
    int32							LightDataOffset;
    float							LightVolumetricScatteringIntensity;
    int32							CameraPayloadOffset;
    int32							SubUVDataOffset;
    int32							SubImages_Horizontal;
    int32							SubImages_Vertical;
    bool						bUseLocalSpace;
    bool						bLockAxis;
    uint8						ScreenAlignment;
    uint8						LockAxisFlag;
    uint8						EmitterRenderMode;
    uint8						EmitterNormalsMode;
    FVector2D					PivotOffset;
    bool						bUseVelocityForMotionBlur;
    bool						bRemoveHMDRoll;
    float						MinFacingCameraBlendDistance;
    float						MaxFacingCameraBlendDistance;

    /** Constructor */
    FDynamicSpriteEmitterReplayDataBase();
    ~FDynamicSpriteEmitterReplayDataBase();

    /** Serialization */
    virtual void Serialize(FArchive& Ar);

};

/** Source data for Mesh emitters */
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

    /** Constructor */
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


    /** Serialization */
    virtual void Serialize(FArchive& Ar)
    {
        // Call parent implementation
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

    /** Custom new/delete with recycling */
    void* operator new(size_t Size); // 제대로 할당 안되면 new연산자 제거.
    void operator delete(void* RawMemory, size_t Size);

    /** Callback from the renderer to gather simple lights that this proxy wants renderered. */
    //virtual void GatherSimpleLights(const FParticleSystemSceneProxy* Proxy, const FSceneViewFamily& ViewFamily, FSimpleLightArray& OutParticleLights) const {}

    /** Returns the source data for this particle system */
    virtual const FDynamicEmitterReplayDataBase& GetSource() const = 0;

    /** Stat id of this object, 0 if nobody asked for it yet */
    //mutable TStatId StatID;
    /** true if this emitter is currently selected */
    uint32	bSelected : 1;
    /** true if this emitter has valid rendering data */
    uint32	bValid : 1;

    int32  EmitterIndex;
};

/** Base class for Sprite emitters and other emitter types that share similar features. */
struct FDynamicSpriteEmitterDataBase : public FDynamicEmitterDataBase
{
    FDynamicSpriteEmitterDataBase(const UParticleModuleRequired* RequiredModule) :
        FDynamicEmitterDataBase(RequiredModule),
        bUsesDynamicParameter(false)
    {
        //MaterialResource = nullptr;
    }

    virtual ~FDynamicSpriteEmitterDataBase()
    {
    }

    /**
     *	Sort the given sprite particles
     *
     *	@param	SorceMode			The sort mode to utilize (EParticleSortMode)
     *	@param	bLocalSpace			true if the emitter is using local space
     *	@param	ParticleCount		The number of particles
     *	@param	ParticleData		The actual particle data
     *	@param	ParticleStride		The stride between entries in the ParticleData array
     *	@param	ParticleIndices		Indirect index list into ParticleData
     *	@param	View				The scene view being rendered
     *	@param	LocalToWorld		The local to world transform of the component rendering the emitter
     *	@param	ParticleOrder		The array to fill in with ordered indices
     */
    void SortSpriteParticles(int32 SortMode, bool bLocalSpace,
        int32 ParticleCount, const uint8* ParticleData, int32 ParticleStride, const uint16* ParticleIndices,
        /*const FSceneView* View,*/ const FMatrix& LocalToWorld, FParticleOrder* ParticleOrder) const;

    /**
     *	Get the vertex stride for the dynamic rendering data
     */
    virtual int32 GetDynamicVertexStride(/*ERHIFeatureLevel::Type *//*InFeatureLevel*/) const
    {
        //checkf(0, TEXT("GetDynamicVertexStride MUST be overridden"));
        return 0;
    }

    /**
     *	Get the vertex stride for the dynamic parameter rendering data
     */
    virtual int32 GetDynamicParameterVertexStride() const
    {
        assert(0, TEXT("GetDynamicParameterVertexStride MUST be overridden"));
        return 0;
    }

    /**
     *	Get the source replay data for this emitter
     */
    virtual const FDynamicSpriteEmitterReplayDataBase* GetSourceData() const
    {
        assert(0, TEXT("GetSourceData MUST be overridden"));
        return NULL;
    }

    /**
     *	Gets the information required for allocating this emitters indices from the global index array.
     */
    virtual void GetIndexAllocInfo(int32& OutNumIndices, int32& OutStride) const
    {
        assert(0, TEXT("GetIndexAllocInfo is not valid for this class."));
    }

    /**
     *	Debug rendering
     *
     *	@param	Proxy		The primitive scene proxy for the emitter.
     *	@param	PDI			The primitive draw interface to render with
     *	@param	View		The scene view being rendered
     *	@param	bCrosses	If true, render Crosses at particle position; false, render points
     */
    //virtual void RenderDebug(const FParticleSystemSceneProxy* Proxy, FPrimitiveDrawInterface* PDI, const FSceneView* View, bool bCrosses) const;

    virtual void DoBufferFill(FAsyncBufferFillData& Me) const
    {
        // Must be overridden if called
        assert(0);
    }

    /** The material render proxy for this emitter */
    //const FMaterialRenderProxy* MaterialResource;
    /** true if the particle emitter utilizes the DynamicParameter module */
    uint32 bUsesDynamicParameter : 1;
};

/** Dynamic emitter data for sprite emitters */
struct FDynamicSpriteEmitterData : public FDynamicSpriteEmitterDataBase
{
    FDynamicSpriteEmitterData(const UParticleModuleRequired* RequiredModule) :
        FDynamicSpriteEmitterDataBase(RequiredModule)
    {
    }

    ~FDynamicSpriteEmitterData()
    {
    }

    /** Initialize this emitter's dynamic rendering data, called after source data has been filled in */
    void Init(bool bInSelected);

    /**
     *	Get the vertex stride for the dynamic rendering data
     */
    virtual int32 GetDynamicVertexStride(/*ERHIFeatureLevel::Type InFeatureLevel*/) const override // ERHIFeatureLevel::Type 은 렌더 플랫폼에 관련된 정보이므로 필요 없음.
    {
        return sizeof(FParticleSpriteVertex);
    }

    /**
     *	Get the vertex stride for the dynamic parameter rendering data
     */
    virtual int32 GetDynamicParameterVertexStride() const override
    {
        return sizeof(FParticleVertexDynamicParameter);
    }

    /**
     *	Get the source replay data for this emitter
     */
    virtual const FDynamicSpriteEmitterReplayDataBase* GetSourceData() const override
    {
        return &Source;
    }

    /**
     *	Retrieve the vertex and (optional) index required to render this emitter.
     *	Render-thread only
     *
     *	@param	VertexData			The memory to fill the vertex data into
     *	@param	FillIndexData		The index data to fill in
     *	@param	ParticleOrder		The (optional) particle ordering to use
     *	@param	InCameraPosition	The position of the camera in world space.
     *	@param	InLocalToWorld		Transform from local to world space.
     *	@param	InstanceFactor		The factor to duplicate instances by.
     *
     *	@return	bool			true if successful, false if failed
     */
    bool GetVertexAndIndexData(void* VertexData, void* DynamicParameterVertexData, void* FillIndexData, FParticleOrder* ParticleOrder, const FVector& InCameraPosition, const FMatrix& InLocalToWorld, uint32 InstanceFactor) const;

    /**
     *	Retrieve the vertex and (optional) index required to render this emitter.
     *  This version for non-instanced platforms.
     *	Render-thread only
     *
     *	@param	VertexData			The memory to fill the vertex data into
     *	@param	FillIndexData		The index data to fill in
     *	@param	ParticleOrder		The (optional) particle ordering to use
     *	@param	InCameraPosition	The position of the camera in world space.
     *	@param	InLocalToWorld		Transform from local to world space.
     *
     *	@return	bool			true if successful, false if failed
     */
    bool GetVertexAndIndexDataNonInstanced(void* VertexData, void* DynamicParameterVertexData, void* FillIndexData, FParticleOrder* ParticleOrder, const FVector& InCameraPosition, const FMatrix& InLocalToWorld, int32 NumVerticesPerParticle) const;

    /** Returns the source data for this particle system */
    virtual const FDynamicEmitterReplayDataBase& GetSource() const override
    {
        return Source;
    }

    /** The frame source data for this particle system.  This is everything needed to represent this
        this particle system frame.  It does not include any transient rendering thread data.  Also, for
        non-simulating 'replay' particle systems, this data may have come straight from disk! */
    FDynamicSpriteEmitterReplayDataBase Source;

};


/** Dynamic emitter data for Mesh emitters */
struct FDynamicMeshEmitterData : public FDynamicSpriteEmitterDataBase
{
    FDynamicMeshEmitterData(const UParticleModuleRequired* RequiredModule);

    virtual ~FDynamicMeshEmitterData();

    /** Initialize this emitter's dynamic rendering data, called after source data has been filled in */
    void Init(bool bInSelected,
        const FParticleMeshEmitterInstance* InEmitterInstance,
        UStaticMesh* InStaticMesh,
        bool InUseStaticMeshLODs,
        float InLODSizeScale
        /*ERHIFeatureLevel::Type InFeatureLevel*/);

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

    /**
     *	Get the vertex stride for the dynamic rendering data
     */
    virtual int32 GetDynamicVertexStride(/*ERHIFeatureLevel::Type*/ /*InFeatureLevel*/) const override
    {
        return sizeof(FMeshParticleInstanceVertex);
    }

    virtual int32 GetDynamicParameterVertexStride() const override
    {
        return sizeof(FMeshParticleInstanceVertexDynamicParameter);
    }

    /**
     *	Get the source replay data for this emitter
     */
    virtual const FDynamicSpriteEmitterReplayDataBase* GetSourceData() const override
    {
        return &Source;
    }

    /** Returns the source data for this particle system */
    virtual const FDynamicEmitterReplayDataBase& GetSource() const override
    {
        return Source;
    }

    /** The frame source data for this particle system.  This is everything needed to represent this
        this particle system frame.  It does not include any transient rendering thread data.  Also, for
        non-simulating 'replay' particle systems, this data may have come straight from disk! */
    FDynamicMeshEmitterReplayData Source;

    int32					LastFramePreRendered;

    UStaticMesh* StaticMesh;
    //TArray<FMaterialRenderProxy*, TInlineAllocator<2>> MeshMaterials;

    /** offset to FMeshTypeDataPayload */
    uint32 MeshTypeDataOffset;

    // 'orientation' items...
    // These don't need to go into the replay data, as they are constant over the life of the emitter
    /** If true, apply the 'pre-rotation' values to the mesh. */
    uint32 bApplyPreRotation : 1;
    /** If true, then use the locked axis setting supplied. Trumps locked axis module and/or TypeSpecific mesh settings. */
    uint32 bUseMeshLockedAxis : 1;
    /** If true, then use the camera facing options supplied. Trumps all other settings. */
    uint32 bUseCameraFacing : 1;
    /**
     *	If true, apply 'sprite' particle rotation about the orientation axis (direction mesh is pointing).
     *	If false, apply 'sprite' particle rotation about the camera facing axis.
     */
    uint32 bApplyParticleRotationAsSpin : 1;
    /**
    *	If true, all camera facing options will point the mesh against the camera's view direction rather than pointing at the cameras location.
    *	If false, the camera facing will point to the cameras position as normal.
    */
    uint32 bFaceCameraDirectionRatherThanPosition : 1;
    /** The EMeshCameraFacingOption setting to use if bUseCameraFacing is true. */
    uint8 CameraFacingOption;

    bool bUseStaticMeshLODs;
    float LODSizeScale;
    mutable int32 LastCalculatedMeshLOD;
    const FParticleMeshEmitterInstance* EmitterInstance;
};
