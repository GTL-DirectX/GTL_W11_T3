#pragma once
#include "IRenderPass.h"
#include "Container/Array.h"
#include "UnrealClient.h"

#include <d3d11.h>
#include <dxgi.h>

#include "Particles/ParticleHelper.h"

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
    virtual void PrepareRenderArr() override;
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    void RenderMeshParticles(
        FDynamicMeshEmitterData* MeshData, UParticleSystemComponent* ParticleSystemComp,
        const std::shared_ptr<FEditorViewportClient>& Viewport
    );
    void RenderSpriteParticles(
        FDynamicSpriteEmitterData* SpriteData, UParticleSystemComponent* PSC,
        const std::shared_ptr<FEditorViewportClient>& Viewport
    );

    virtual void ClearRenderArr() override;

private:
    void CreateRenderStates();
    void CreateShader();
    void CreateBuffers();

    void PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport);

private:
    TArray<FParticleRenderEntry> ParticleRenderEntries;
    TArray<UParticleSystemComponent*> ParticleSystemComponents;
    
    FDXDBufferManager* BufferManager;
    FGraphicsDevice* Graphics;
    FDXDShaderManager* ShaderManager;

    ID3D11VertexShader* VertexShader;
    ID3D11PixelShader* PixelShader;
    ID3D11InputLayout* InputLayout;

    ID3D11BlendState* AlphaBlendState = nullptr;
    ID3D11BlendState* AdditiveBlendState = nullptr;
};
