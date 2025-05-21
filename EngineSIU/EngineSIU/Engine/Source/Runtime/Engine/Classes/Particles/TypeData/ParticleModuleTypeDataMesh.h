#include "Particles/TypeData/ParticleModuleTypeDataBase.h"

class UStaticMesh;

class UParticleModuleTypeDataMesh : public UParticleModuleTypeDataBase
{
    DECLARE_CLASS(UParticleModuleTypeDataMesh, UParticleModuleTypeDataBase)

public:
    UParticleModuleTypeDataMesh() = default;
    virtual void PostInitProperties() override;
    virtual UObject* Duplicate(UObject* InOuter) override;

    UStaticMesh* Mesh;
};
