#include "ParticleRenderPass.h"

#include "RendererHelpers.h"
#include "Components/Mesh/StaticMeshRenderData.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleHelper.h"
#include "Particles/ParticleSystemComponent.h"
#include "UObject/UObjectIterator.h"

#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"

#include "LevelEditor/SLevelEditor.h"
#include "Particles/ParticleEmitterInstances.h"
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
    alphaRT.DestBlend = D3D11_BLEND_ONE;
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
    DepthStencilStateDesc.DepthEnable = TRUE;
    DepthStencilStateDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    DepthStencilStateDesc.DepthFunc = D3D11_COMPARISON_LESS;

    // Stencil test parameters
    DepthStencilStateDesc.StencilEnable = FALSE;
    //DepthStencilStateDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    //DepthStencilStateDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

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

    hr = ShaderManager->AddVertexShaderAndInputLayout(
        L"ParticleSpriteVertexShader", L"Shaders/ParticleSpriteVertexShader.hlsl", "mainVS", FParticleSpriteVertex::LayoutDesc,
        ARRAYSIZE(FParticleSpriteVertex::LayoutDesc)
    );

    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to load ParticleSpriteVertexShader and InputLayout"));
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
    BufferManager->CreateStructuredBufferGeneric<FBaseParticle>(
        "ParticleSpriteInstanceBuffer", nullptr, 1000, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE
    );
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

    VertexShader = ShaderManager->GetVertexShaderByKey(L"ParticleSpriteVertexShader");
    InputLayout = ShaderManager->GetInputLayoutByKey(L"ParticleSpriteVertexShader");
    PixelShader = ShaderManager->GetPixelShaderByKey(L"ParticleSpritePixelShader");

    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);
    Graphics->DeviceContext->IASetInputLayout(InputLayout);

    BufferManager->BindConstantBuffer(TEXT("FMaterialConstants"), 0, EShaderStage::Pixel);
}

void FParticleRenderPass::CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    Graphics->DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilState_Default, 1);
}

void FParticleRenderPass::PrepareRenderArr()
{
    ParticleSystemComponents.Empty();

    for (const auto& Component : TObjectRange<UParticleSystemComponent>())
    {
        if (Component->GetOwner() && !Component->GetOwner()->IsHidden() && Component->GetWorld() == GEngine->ActiveWorld)
        {
            ParticleSystemComponents.Add(Component);
            for (FDynamicEmitterDataBase* EmitterRenderData : Component->GetRenderData())
            {
                FDynamicSpriteEmitterData* SpriteEmitterData = static_cast<FDynamicSpriteEmitterData*>(EmitterRenderData);
                SpriteEmitterData->Init(SpriteEmitterData->bSelected);
            }
        }
    }

    ParticleSystemComponents.Sort(
        [](const UParticleSystemComponent* A, const UParticleSystemComponent* B)
        {
            const FVector LocA = A->GetWorldLocation();
            const FVector LocB = B->GetWorldLocation();
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

    for (UParticleSystemComponent* ParticleSystemComp : ParticleSystemComponents)
    {
        //FDynamicEmitterDataBase* EmitterData = Entry.EmitterData;
        //if (!EmitterData || !EmitterData->bValid)
        //    continue;

        for (FDynamicEmitterDataBase* EmitterBase : ParticleSystemComp->GetRenderData())
        {
            if (!EmitterBase || !EmitterBase->bValid)
                continue;

            if (FDynamicMeshEmitterData* MeshData = dynamic_cast<FDynamicMeshEmitterData*>(EmitterBase))
            {
                RenderMeshParticles(MeshData, ParticleSystemComp, Viewport);
            }
            else if (FDynamicSpriteEmitterData* SpriteData = dynamic_cast<FDynamicSpriteEmitterData*>(EmitterBase))
            {
                RenderSpriteParticles(SpriteData, ParticleSystemComp, Viewport);
            }

        }
    }
}

void FParticleRenderPass::RenderMeshParticles(
    FDynamicMeshEmitterData* MeshData,
    UParticleSystemComponent* ParticleSystemComp,
    const std::shared_ptr<FEditorViewportClient>& Viewport)
{

}

void FParticleRenderPass::RenderSpriteParticles(
    FDynamicSpriteEmitterData* SpriteData,
    UParticleSystemComponent* ParticleSystemComp,
    const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    // 파티클 수 가져오기 (ReplayData나 GetParticleCount() 사용)
    const int32 NumParticles = SpriteData->Source.ActiveParticleCount;
    if (NumParticles == 0)
        return;

    // 버텍스 배열 준비 (파티클당 4개 꼭짓점)
    TArray<FParticleSpriteVertex> SpriteVertices;
    SpriteVertices.Reserve(NumParticles * 4);
    SpriteVertices.AddUninitialized(NumParticles * 4);

    // 핵심: 여기서 GetVertexAndIndexData() 호출
    SpriteData->GetVertexAndIndexData(
        /*VertexData=*/ SpriteVertices.GetData(),
        /*DynamicParameterVertexData=*/ nullptr,
        /*FillIndexData=*/ nullptr,
        /*ParticleOrder=*/ nullptr,
        /*InCameraPosition=*/ Viewport->GetCameraLocation(),
        /*InLocalToWorld=*/ ParticleSystemComp->EmitterInstances[0]->SimulationToWorld,
        /*InstanceFactor=*/ 1
    );

    SpriteVertices.Sort(
        [Viewport](const FParticleSpriteVertex& A, const FParticleSpriteVertex& B)
        {
            const FVector LocA = A.Position;
            const FVector LocB = B.Position;
            const FVector LocCam = Viewport->GetCameraLocation();

            const float DistA = (Viewport->GetCameraLocation() - A.Position).SquaredLength();
            const float DistB = (Viewport->GetCameraLocation() - B.Position).SquaredLength();
            return DistA > DistB;
        }
    );

    if (UMaterial* Material = SpriteData->Source.Material)
    {
        const FMaterialInfo& MaterialInfo = Material->GetMaterialInfo();
        MaterialUtils::UpdateMaterial(BufferManager, Graphics, MaterialInfo);
    }

    BufferManager->UpdateStructuredBuffer("ParticleSpriteInstanceBuffer", SpriteVertices);
    BufferManager->BindStructuredBufferSRV("ParticleSpriteInstanceBuffer", 60, EShaderStage::Vertex);
    Graphics->DeviceContext->DrawInstanced(
        /*VertexCountPerInstance=*/ 6,
        /*InstanceCount=*/ SpriteVertices.Num(),
        /*StartVertexLocation=*/ 0,
        /*StartInstanceLocation=*/ 0
    );
}


void FParticleRenderPass::ClearRenderArr()
{
    ParticleRenderEntries.Empty();
}
