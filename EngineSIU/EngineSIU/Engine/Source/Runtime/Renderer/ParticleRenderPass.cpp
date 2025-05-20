#include "ParticleRenderPass.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleHelper.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/UObjectIterator.h"

#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"

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
    CreateRenderStates();
    CreateShader();
    CreateBuffers();
}

void FParticleRenderPass::CreateRenderStates()
{
    HRESULT hr = S_OK; 
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

    hr = Graphics->Device->CreateBlendState(&alphaDesc, &AlphaBlendState);
    if (FAILED(hr))
    {
        // 오류 처리
        UE_LOG(ELogLevel::Error, TEXT("Failed to create AlphaBlendState"));
    }

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

    hr = Graphics->Device->CreateBlendState(&addDesc, &AdditiveBlendState);
    if (FAILED(hr))
    {
        // 오류 처리
        UE_LOG(ELogLevel::Error, TEXT("Failed to create AdditiveBlendState"));
    }


    D3D11_DEPTH_STENCIL_DESC DepthStencilStateDesc = {};

    // Depth test parameters
    DepthStencilStateDesc.DepthEnable = true;
    DepthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    DepthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;

    // Stencil test parameters
    DepthStencilStateDesc.StencilEnable = true;
    DepthStencilStateDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    DepthStencilStateDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

    // Stencil operations if pixel is front-facing
    DepthStencilStateDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    DepthStencilStateDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
    DepthStencilStateDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    DepthStencilStateDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    // Stencil operations if pixel is back-facing
    DepthStencilStateDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
    DepthStencilStateDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
    DepthStencilStateDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
    DepthStencilStateDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

    //// DepthStencil 상태 생성
    
    hr = Graphics->Device->CreateDepthStencilState(&DepthStencilStateDesc, &Graphics->DepthStencilState_DepthWriteDisabled);
    if (FAILED(hr))
    {
        // 오류 처리
        return;
    }

}

void FParticleRenderPass::CreateShader()
{
    HRESULT hr = S_OK;

    hr = ShaderManager->AddVertexShader(L"ParticleSpriteVertexShader", L"Shaders/ParticleSpriteVertexShader.hlsl", "mainVS", nullptr);

    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to load ParticleSpriteVertexShader"));
    }

    hr = ShaderManager->AddPixelShader(L"ParticleSpritePixelShader", L"Shaders/ParticleSpritePixelShader.hlsl", "mainPS", nullptr);
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to load ParticleSpritePixelShader"));
    }
}

void FParticleRenderPass::CreateBuffers()
{
    if (!BufferManager)
    {
        UE_LOG(ELogLevel::Error, TEXT("BufferManager is null"));
        return;
    }

    BufferManager->CreateStructuredBufferGeneric<FBaseParticle>("ParticleSpriteInstanceBuffer", nullptr, 1000, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

}

void FParticleRenderPass::PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->RSSetViewports(1, &Viewport->GetViewportResource()->GetD3DViewport());

    constexpr EResourceType ResourceType = EResourceType::ERT_Scene;
    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    const FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(ResourceType);
    const FDepthStencilRHI* DepthStencilRHI = ViewportResource->GetDepthStencil(ResourceType);

    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, DepthStencilRHI->DSV);

    Graphics->DeviceContext->OMSetBlendState(AlphaBlendState, nullptr, 0xffffffff);
    Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilState_DepthWriteDisabled, 1);

    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    ID3D11VertexShader* VertexShader = ShaderManager->GetVertexShaderByKey(L"ParticleSpriteVertexShader");
    ID3D11PixelShader* PixelShader = ShaderManager->GetPixelShaderByKey(L"ParticleSpritePixelShader");
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(nullptr);

}

void FParticleRenderPass::CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    Graphics->DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilState_Default, 1);
}

void FParticleRenderPass::PrepareRenderArr()
{
    ParticleRenderEntries.Empty();

    for (const auto& Component : TObjectRange<UParticleSystemComponent>())
    {
        if (Component->GetOwner() && !Component->GetOwner()->IsHidden() && Component->GetWorld() == GEngine->ActiveWorld)
        {
            ParticleRenderEntries.Add({ Component, nullptr });

            /*for (FDynamicEmitterDataBase* EmitterData : Component->GetRenderData())
            {
                if (EmitterData)
                {
                    ParticleRenderEntries.Add({Component, EmitterData});
                }
            }*/
        }
    }

    ParticleRenderEntries.Sort([](const FParticleRenderEntry& A, const FParticleRenderEntry& B)
        {
            const FVector LocA = A.Component->GetWorldLocation();
            const FVector LocB = B.Component->GetWorldLocation();
            const FVector LocCam = GEngineLoop.GetLevelEditor()->GetActiveViewportClient()->GetCameraLocation();

            const float DistA = (LocCam - LocA).SquaredLength();
            const float DistB = (LocCam - LocB).SquaredLength();

            return DistA > DistB;
        }
    );

}

void FParticleRenderPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    PrepareRender(Viewport);
    /*if (ParticleRenderEntries.IsEmpty())
        return;*/

    for (const FParticleRenderEntry& Entry : ParticleRenderEntries)
    {
        /*FDynamicEmitterDataBase* EmitterData = Entry.EmitterData;
        if (!EmitterData || !EmitterData->bValid)
            continue;*/
    

        TArray<FParticleSpriteVertex> SpriteVertices = {
            { FVector(0.f), 0.f, FVector(0.f), 0.f, FVector2D(1.f), 0.f, 0.f, FLinearColor(0.f, 0.f, 0.f, 1.f) },
            { FVector(1.f), 0.f, FVector(0.f), 0.f, FVector2D(0.5f), 0.f, 0.f, FLinearColor(0.f, 0.f, 0.f, 1.f) },
            { FVector(2.f), 0.f, FVector(0.f), 0.f, FVector2D(0.25f), 0.f, 0.f, FLinearColor(0.f, 0.f, 0.f, 1.f) },
        };

        SpriteVertices.Sort(
            [Viewport](const FParticleSpriteVertex& A, const FParticleSpriteVertex& B)
            {
                const FVector LocA = A.Position;
                const FVector LocB = B.Position;
                const FVector LocCam = Viewport->GetCameraLocation();

                const float DistA = (LocCam - LocA).SquaredLength();
                const float DistB = (LocCam - LocB).SquaredLength();

                return DistA > DistB;
            }
        );

        BufferManager->UpdateStructuredBuffer("ParticleSpriteInstanceBuffer", SpriteVertices);
        BufferManager->BindStructuredBufferSRV("ParticleSpriteInstanceBuffer", 0, EShaderStage::Vertex);

        Graphics->DeviceContext->DrawInstanced(6, SpriteVertices.Num(), 0, 0);
    }
    
}

void FParticleRenderPass::ClearRenderArr()
{
    ParticleRenderEntries.Empty();
}
