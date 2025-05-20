#pragma once
#include "IRenderPass.h"
#include "Container/Array.h"
#include "UnrealClient.h"

struct FDynamicEmitterDataBase;
class UParticleSystemComponent;

// 각 Emitter마다 렌더링 방식이 다를 수 있으므로 (Component, EmitterData) 쌍으로 관리
struct FParticleRenderEntry
{
    UParticleSystemComponent* Component;
    FDynamicEmitterDataBase*  EmitterData;
};

class FParticleRenderPass : public IRenderPass
{
public:
    FParticleRenderPass();
    virtual ~FParticleRenderPass() override;

    void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager) override;
    void CreateBlendState();
    virtual void PrepareRenderArr() override;
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    virtual void ClearRenderArr() override;

private:
    TArray<FParticleRenderEntry> ParticleRenderEntries;
    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;

    ID3D11BlendState* AlphaBlendState = nullptr;
    ID3D11BlendState* AdditiveBlendState = nullptr;
};
