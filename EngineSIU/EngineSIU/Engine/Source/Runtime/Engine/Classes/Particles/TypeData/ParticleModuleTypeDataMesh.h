#include "Particles/TypeData/ParticleModuleTypeDataBase.h"

class UStaticMesh;

class UParticleModuleTypeDataMesh : public UParticleModuleTypeDataBase
{
    DECLARE_CLASS(UParticleModuleTypeDataMesh, UParticleModuleTypeDataBase)

public:
    UParticleModuleTypeDataMesh() = default;

    UStaticMesh* Mesh;
};
