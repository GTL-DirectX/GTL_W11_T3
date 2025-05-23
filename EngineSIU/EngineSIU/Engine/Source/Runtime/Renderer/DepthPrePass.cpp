#include "DepthPrePass.h"

#include "HAL/PlatformType.h"
#include "UnrealClient.h"
#include "D3D11RHI/DXDBufferManager.h"
#include "D3D11RHI/GraphicDevice.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "UnrealEd/EditorViewportClient.h"

#define SAFE_RELEASE(p) if (p) { p->Release(); p = nullptr; }

FDepthPrePass::FDepthPrePass()
{
}

FDepthPrePass::~FDepthPrePass()
{
}   

void FDepthPrePass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
    FStaticMeshRenderPass::Initialize(InBufferManager, InGraphics, InShaderManage);
    FSkeletalMeshRenderPass::Initialize(InBufferManager, InGraphics, InShaderManage);
}

void FDepthPrePass::PrepareRenderArr()
{
    
    //FStaticMeshRenderPass::PrepareRenderArr();
    //FSkeletalMeshRenderPass::PrepareRenderArr();
}

void FDepthPrePass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    PrepareRenderState(Viewport);
    FStaticMeshRenderPass::PrepareRenderArr();
    FStaticMeshRenderPass::RenderAllStaticMeshes(Viewport);
    FStaticMeshRenderPass::ClearRenderArr();

    FSkeletalMeshRenderPass::Render(Viewport);
    // 렌더 타겟 해제
    FStaticMeshRenderPass::Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void FDepthPrePass::ClearRenderArr()
{
    //FStaticMeshRenderPass::ClearRenderArr();
}

void FDepthPrePass::PrepareRenderState(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    /*
    // A. 컬러 쓰기 비활성화 (깊이만 기록)
    FLOAT zeroColor[4] = { 0, 0, 0, 0 };
    Graphics->DeviceContext->ClearDepthStencilView(DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
    
    Graphics->DeviceContext->OMSetBlendState(nullptr, zeroColor, 0xFFFFFFFF); // 또는 Custom DisableBlendState
    // B. Depth-only Rasterizer & DepthStencil 설정
    Graphics->DeviceContext->OMSetDepthStencilState(DepthStencilState_OnlyWrite, 0);  // 깊이만 기록, 테스트 ON

    // C. 컬러 렌더 타겟 없음, 깊이만
    ID3D11RenderTargetView* nullRTV = nullptr;
    Graphics->DeviceContext->OMSetRenderTargets(1, &nullRTV, DepthStencilView); // ← 깊이 전용
    */
    
    ID3D11VertexShader* VertexShader = FStaticMeshRenderPass::ShaderManager->GetVertexShaderByKey(L"StaticMeshVertexShader");
    ID3D11InputLayout* InputLayout = FStaticMeshRenderPass::ShaderManager->GetInputLayoutByKey(L"StaticMeshVertexShader");
    
    FStaticMeshRenderPass::Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    FStaticMeshRenderPass::Graphics->DeviceContext->IASetInputLayout(InputLayout);

    // 뎁스만 필요하므로, 픽셀 쉐이더는 지정 안함.
    FStaticMeshRenderPass::Graphics->DeviceContext->PSSetShader(nullptr, nullptr, 0);

    FStaticMeshRenderPass::Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    FStaticMeshRenderPass::Graphics->DeviceContext->RSSetState(FStaticMeshRenderPass::Graphics->RasterizerSolidBack);

    FStaticMeshRenderPass::Graphics->DeviceContext->OMSetBlendState(nullptr, nullptr, 0xFFFFFFFF);

    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    FDepthStencilRHI* DepthStencilRHI = ViewportResource->GetDepthStencil(EResourceType::ERT_Debug);

    FStaticMeshRenderPass::Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, DepthStencilRHI->DSV); // ← 깊이 전용
}
