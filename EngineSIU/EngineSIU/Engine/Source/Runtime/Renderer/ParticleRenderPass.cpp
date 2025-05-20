#include "ParticleRenderPass.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/UObjectIterator.h"

FParticleRenderPass::FParticleRenderPass()
    : BufferManager(nullptr)
    , Graphics(nullptr)
    , ShaderManager(nullptr)
{
}

FParticleRenderPass::~FParticleRenderPass()
{
    if (AlphaBlendState)
    {
        AlphaBlendState->Release();
        AlphaBlendState = nullptr;
    }
    if (AdditiveBlendState)
    {
        AdditiveBlendState->Release();
        AdditiveBlendState = nullptr;
    }
}

void FParticleRenderPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManager)
{
    BufferManager = InBufferManager;
    Graphics = InGraphics;
    ShaderManager = InShaderManager;
    CreateShader();
    CreateBlendState();
}

void FParticleRenderPass::CreateShader()
{
    D3D11_INPUT_ELEMENT_DESC ParticleSpriteLayoutDesc[] =
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
}

void FParticleRenderPass::CreateBlendState()
{
    // 알파 블렌딩
    D3D11_BLEND_DESC alphaDesc = {};
    alphaDesc.AlphaToCoverageEnable = FALSE;
    alphaDesc.IndependentBlendEnable = FALSE;
    auto& alphaRT = alphaDesc.RenderTarget[0];
    alphaRT.BlendEnable = TRUE;
    alphaRT.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    alphaRT.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    alphaRT.BlendOp = D3D11_BLEND_OP_ADD;
    alphaRT.SrcBlendAlpha = D3D11_BLEND_ONE;
    alphaRT.DestBlendAlpha = D3D11_BLEND_ZERO;
    alphaRT.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    alphaRT.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    Graphics->Device->CreateBlendState(&alphaDesc, &AlphaBlendState);

    // 가산(애디티브) 블렌딩
    D3D11_BLEND_DESC addDesc = {};
    addDesc.AlphaToCoverageEnable = FALSE;
    addDesc.IndependentBlendEnable = FALSE;
    auto& addRT = addDesc.RenderTarget[0];
    addRT.BlendEnable = TRUE;
    addRT.SrcBlend = D3D11_BLEND_SRC_ALPHA;
    addRT.DestBlend = D3D11_BLEND_ONE;
    addRT.BlendOp = D3D11_BLEND_OP_ADD;
    addRT.SrcBlendAlpha = D3D11_BLEND_ZERO;
    addRT.DestBlendAlpha = D3D11_BLEND_ONE;
    addRT.BlendOpAlpha = D3D11_BLEND_OP_ADD;
    addRT.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    Graphics->Device->CreateBlendState(&addDesc, &AdditiveBlendState);
}

void FParticleRenderPass::PrepareRenderArr()
{
    ParticleRenderEntries.Empty();

    for (const auto& Component : TObjectRange<UParticleSystemComponent>())
    {
        if (Component->GetOwner() && !Component->GetOwner()->IsHidden() && Component->GetWorld() == GEngine->ActiveWorld)
        {
            for (FDynamicEmitterDataBase* EmitterData : Component->GetRenderData())
            {
                if (EmitterData)
                {
                    ParticleRenderEntries.Add({Component, EmitterData});
                }
            }
        }
    }
}

void FParticleRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
}

void FParticleRenderPass::ClearRenderArr()
{
    ParticleRenderEntries.Empty();
}
