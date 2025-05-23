#pragma once
#define _TCHAR_DEFINED
#include "Define.h"
#include <d3d11.h>
#include <d3dcompiler.h>
#include "Container/String.h"
#include "Container/Array.h"
#include "Container/Map.h"
#include "Engine/Texture.h"
#include "GraphicDevice.h"
#include "UserInterface/Console.h"

// ShaderStage 열거형
enum class EShaderStage
{
    Vertex,
    Pixel,
    Compute,
    Geometry,
};
struct QuadVertex
{
    float Position[3];
    float TexCoord[2];
};

class FDXDBufferManager
{
public:
    QuadVertex Q;

    FDXDBufferManager() = default;
    void Initialize(ID3D11Device* DXDevice, ID3D11DeviceContext* DXDeviceContext);

    // 템플릿을 활용한 버텍스 버퍼 생성 (정적/동적) - FString / FWString
    template<typename T>
    HRESULT CreateVertexBuffer(const FString& KeyName, const TArray<T>& vertices, FVertexInfo& OutVertexInfo, D3D11_USAGE Usage = D3D11_USAGE_DEFAULT, UINT CpuAccessFlags = 0);
    template<typename T>
    HRESULT CreateVertexBuffer(const FWString& KeyName, const TArray<T>& vertices, FVertexInfo& OutVertexInfo, D3D11_USAGE Usage = D3D11_USAGE_DEFAULT, UINT CpuAccessFlags = 0);
    template<typename T>
    HRESULT CreateVertexBuffer(const FString& KeyName, const TArray<T>& vertices, D3D11_USAGE Usage = D3D11_USAGE_DEFAULT, UINT CpuAccessFlags = 0);
    template<typename T>
    HRESULT CreateVertexBuffer(const FWString& KeyName, const TArray<T>& vertices, D3D11_USAGE Usage = D3D11_USAGE_DEFAULT, UINT CpuAccessFlags = 0);
    template<typename T>
    HRESULT CreateVertexBuffer(const FString& KeyName, const void* Vertices, uint32 ByteWidth, uint32 Stride, uint32 Offset = 0, D3D11_USAGE Usage = D3D11_USAGE_DEFAULT, UINT CpuAccessFlags = 0);

    template<typename T>
    HRESULT CreateIndexBuffer(const FString& KeyName, const TArray<T>& indices, FIndexInfo& OutIndexInfo, D3D11_USAGE Usage = D3D11_USAGE_DEFAULT, UINT CpuAccessFlags = 0);
    template<typename T>
    HRESULT CreateIndexBuffer(const FWString& KeyName, const TArray<T>& indices, FIndexInfo& OutIndexInfo, D3D11_USAGE Usage = D3D11_USAGE_DEFAULT, UINT CpuAccessFlags = 0);
    template<typename T>
    HRESULT CreateIndexBuffer(const FWString& KeyName, const TArray<T>& indices, D3D11_USAGE Usage = D3D11_USAGE_DEFAULT, UINT CpuAccessFlags = 0);
    template<typename T>
    HRESULT CreateIndexBuffer(const FString& KeyName, const TArray<T>& indices, D3D11_USAGE Usage = D3D11_USAGE_DEFAULT, UINT CpuAccessFlags = 0);

    template<typename T>
    HRESULT CreateDynamicVertexBuffer(const FString& KeyName, const TArray<T>& vertices, FVertexInfo& OutVertexInfo);

    // 템플릿 헬퍼 함수: 내부에서 버퍼 생성 로직 통합
    template<typename T>
    HRESULT CreateVertexBufferInternal(const FString& KeyName, const TArray<T>& vertices, FVertexInfo& OutVertexInfo,
        D3D11_USAGE usage, UINT cpuAccessFlags);

    template<typename T>
    HRESULT CreateVertexBufferInternal(const FWString& KeyName, const TArray<T>& vertices, FVertexInfo& OutVertexInfo,
        D3D11_USAGE usage, UINT cpuAccessFlags);

    HRESULT CreateUnicodeTextBuffer(const FWString& Text, FBufferInfo& OutBufferInfo, float BitmapWidth, float BitmapHeight, float ColCount, float RowCount);

    void SetStartUV(wchar_t hangul, FVector2D& UVOffset);
    
    void ReleaseBuffers();
    void ReleaseConstantBuffer();
    void ReleaseStructuredBuffer();

    template<typename T>
    HRESULT CreateBufferGeneric(const FString& KeyName, T* data, UINT byteWidth, UINT bindFlags, D3D11_USAGE usage, UINT cpuAccessFlags);

    template<typename T>
    HRESULT CreateStructuredBufferGeneric(const FString& KeyName, T* Data, int32 NumElements, D3D11_USAGE Usage, UINT CpuAccessFlags);

    template<typename T>
    HRESULT CreateConstantBuffer(const FString& KeyName, UINT NumElements = 1);

    template<typename T>
    void UpdateConstantBuffer(const FString& key, const T& data) const;

    template<typename T>
    void UpdateConstantBuffer(const FString& key, const TArray<T>& data) const;

    template<typename T>
    void UpdateStructuredBuffer(const FString& Key, const TArray<T>& Data) const;

    template<typename T>
    void UpdateDynamicVertexBuffer(const FString& KeyName, const TArray<T>& vertices) const;

    void BindConstantBuffers(const TArray<FString>& Keys, UINT StartSlot, EShaderStage Stage) const;
    void BindConstantBuffer(const FString& Key, UINT StartSlot, EShaderStage Stage) const;
    void BindStructuredBufferSRV(const FString& Key, UINT StartSlot, EShaderStage Stage) const;

    template<typename T>
    static void SafeRelease(T*& comObject);

    // Device Context에 버퍼 바인딩
    void SetVertexBuffer(const FString& InName, ID3D11DeviceContext* DeviceContext = nullptr);
    void SetIndexBuffer(const FString& InName, ID3D11DeviceContext* DeviceContext = nullptr);

    FVertexInfo GetVertexBuffer(const FString& InName) const;
    FIndexInfo GetIndexBuffer(const FString& InName) const;
    FVertexInfo GetTextVertexBuffer(const FWString& InName) const;
    FIndexInfo GetTextIndexBuffer(const FWString& InName) const;
    ID3D11Buffer* GetConstantBuffer(const FString& InName) const;
    ID3D11Buffer* GetStructuredBuffer(const FString& InName) const;
    ID3D11ShaderResourceView* GetStructuredBufferSRV(const FString& InName) const;

    void GetQuadBuffer(FVertexInfo& OutVertexInfo, FIndexInfo& OutIndexInfo);
    void GetTextBuffer(const FWString& Text, FVertexInfo& OutVertexInfo, FIndexInfo& OutIndexInfo);
    void CreateQuadBuffer();
private:
    // 16바이트 정렬
    inline UINT Align16(UINT size) { return (size + 15) & ~15; }
private:
    ID3D11Device* DXDevice = nullptr;
    ID3D11DeviceContext* DXDeviceContext = nullptr;

    TMap<FString, FVertexInfo> VertexBufferPool;
    TMap<FString, FIndexInfo> IndexBufferPool;
    TMap<FString, ID3D11Buffer*> ConstantBufferPool;

    TMap<FString, ID3D11Buffer*> StructuredBufferPool;
    TMap<FString, ID3D11ShaderResourceView*> StructuredBufferSRVPool;
    TMap<FString, int32> StructuredBufferElementSizePool;

    TMap<FWString, FBufferInfo> TextAtlasBufferPool;
    TMap<FWString, FVertexInfo> TextAtlasVertexBufferPool;
    TMap<FWString, FIndexInfo> TextAtlasIndexBufferPool;
};

// 템플릿 함수 구현부

template<typename T>
HRESULT FDXDBufferManager::CreateVertexBufferInternal(const FString& KeyName, const TArray<T>& vertices, FVertexInfo& OutVertexInfo,
    D3D11_USAGE usage, UINT cpuAccessFlags)
{
    if (!KeyName.IsEmpty() && VertexBufferPool.Contains(KeyName))
    {
        OutVertexInfo = VertexBufferPool[KeyName];
        return S_OK;
    }
    uint32_t Stride = sizeof(T);
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = usage;
    bufferDesc.ByteWidth = Stride * vertices.Num();
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = cpuAccessFlags;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices.GetData();

    ID3D11Buffer* NewBuffer = nullptr;
    HRESULT hr = DXDevice->CreateBuffer(&bufferDesc, &initData, &NewBuffer);
    if (FAILED(hr))
        return hr;

    OutVertexInfo.NumVertices = static_cast<uint32>(vertices.Num());
    OutVertexInfo.VertexBuffer = NewBuffer;
    OutVertexInfo.Stride = Stride;
    OutVertexInfo.Offset = 0;
    VertexBufferPool.Add(KeyName, OutVertexInfo);

    return S_OK;
}
template<typename T>
HRESULT FDXDBufferManager::CreateIndexBuffer(const FString& KeyName, const TArray<T>& indices, FIndexInfo& OutIndexInfo, D3D11_USAGE Usage, UINT CpuAccessFlags)
{
    if (!KeyName.IsEmpty() && IndexBufferPool.Contains(KeyName))
    {
        OutIndexInfo = IndexBufferPool[KeyName];
        return S_OK;
    }

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = Usage;
    indexBufferDesc.ByteWidth = indices.Num() * sizeof(uint32);
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = CpuAccessFlags;

    D3D11_SUBRESOURCE_DATA indexInitData = {};
    indexInitData.pSysMem = indices.GetData();

    ID3D11Buffer* NewBuffer = nullptr;
    HRESULT hr = DXDevice->CreateBuffer(&indexBufferDesc, &indexInitData, &NewBuffer);
    if (FAILED(hr))
        return hr;

    OutIndexInfo.NumIndices = static_cast<uint32>(indices.Num());
    OutIndexInfo.IndexBuffer = NewBuffer;
    IndexBufferPool.Add(KeyName, FIndexInfo(static_cast<uint32>(indices.Num()), NewBuffer));


    return S_OK;
}

template<typename T>
HRESULT FDXDBufferManager::CreateVertexBuffer(const FString& KeyName, const TArray<T>& vertices, FVertexInfo& OutVertexInfo, D3D11_USAGE Usage, UINT CpuAccessFlags)
{
    return CreateVertexBufferInternal(KeyName, vertices, OutVertexInfo, Usage, CpuAccessFlags);
}


// FWString 전용 버텍스 버퍼 생성 (내부)
template<typename T>
HRESULT FDXDBufferManager::CreateVertexBufferInternal(const FWString& KeyName, const TArray<T>& vertices, FVertexInfo& OutVertexInfo,
    D3D11_USAGE usage, UINT cpuAccessFlags)
{
    if (!KeyName.empty() && TextAtlasVertexBufferPool.Contains(KeyName))
    {
        OutVertexInfo = TextAtlasVertexBufferPool[KeyName];
        return S_OK;
    }
    uint32_t Stride = sizeof(T);
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = usage;
    bufferDesc.ByteWidth = Stride * vertices.Num();
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = cpuAccessFlags;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices.GetData();

    ID3D11Buffer* NewBuffer = nullptr;
    HRESULT hr = DXDevice->CreateBuffer(&bufferDesc, &initData, &NewBuffer);
    if (FAILED(hr))
        return hr;

    OutVertexInfo.NumVertices = static_cast<uint32>(vertices.Num());
    OutVertexInfo.VertexBuffer = NewBuffer;
    OutVertexInfo.Stride = Stride;
    OutVertexInfo.Offset = 0;
    TextAtlasVertexBufferPool.Add(KeyName, OutVertexInfo);

    return S_OK;
}

// FWString 전용 인덱스 버퍼 생성
template<typename T>
HRESULT FDXDBufferManager::CreateIndexBuffer(const FWString& KeyName, const TArray<T>& indices, FIndexInfo& OutIndexInfo, D3D11_USAGE Usage, UINT CpuAccessFlags)
{
    if (!KeyName.empty() && TextAtlasIndexBufferPool.Contains(KeyName))
    {
        OutIndexInfo = TextAtlasIndexBufferPool[KeyName];
        return S_OK;
    }

    D3D11_BUFFER_DESC indexBufferDesc = {};
    indexBufferDesc.Usage = Usage;
    indexBufferDesc.ByteWidth = indices.Num() * sizeof(uint32);
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = CpuAccessFlags;

    D3D11_SUBRESOURCE_DATA indexInitData = {};
    indexInitData.pSysMem = indices.GetData();

    ID3D11Buffer* NewBuffer = nullptr;
    HRESULT hr = DXDevice->CreateBuffer(&indexBufferDesc, &indexInitData, &NewBuffer);
    if (FAILED(hr))
        return hr;

    OutIndexInfo.NumIndices = static_cast<uint32>(indices.Num());
    OutIndexInfo.IndexBuffer = NewBuffer;
    TextAtlasIndexBufferPool.Add(KeyName, FIndexInfo(static_cast<uint32>(indices.Num()), NewBuffer));

    return S_OK;
}

template<typename T>
inline HRESULT FDXDBufferManager::CreateIndexBuffer(const FWString& KeyName, const TArray<T>& indices, D3D11_USAGE Usage, UINT CpuAccessFlags)
{
    FIndexInfo _temp;
    return CreateIndexBuffer(KeyName, indices, _temp, Usage, CpuAccessFlags);
}

template<typename T>
inline HRESULT FDXDBufferManager::CreateIndexBuffer(const FString& KeyName, const TArray<T>& indices, D3D11_USAGE Usage, UINT CpuAccessFlags)
{
    FIndexInfo _temp;
    return CreateIndexBuffer(KeyName, indices, _temp, Usage, CpuAccessFlags);
}

template<typename T>
HRESULT FDXDBufferManager::CreateVertexBuffer(const FWString& KeyName, const TArray<T>& vertices, FVertexInfo& OutVertexInfo, D3D11_USAGE Usage, UINT CpuAccessFlags)
{
    return CreateVertexBufferInternal(KeyName, vertices, OutVertexInfo, Usage, CpuAccessFlags);
}

template<typename T>
inline HRESULT FDXDBufferManager::CreateVertexBuffer(const FWString& KeyName, const TArray<T>& vertices, D3D11_USAGE Usage, UINT CpuAccessFlags)
{
    FVertexInfo _temp;
    return CreateVertexBufferInternal(KeyName, vertices, _temp, Usage, CpuAccessFlags);
}

template<typename T>
inline HRESULT FDXDBufferManager::CreateVertexBuffer(const FString& KeyName, const void* Vertices, uint32 ByteWidth, uint32 Stride, uint32 Offset, D3D11_USAGE Usage, UINT CpuAccessFlags)
{
    if (!KeyName.IsEmpty() && VertexBufferPool.Contains(KeyName))
    {
        return S_OK;
    }
    uint32_t Stride = sizeof(T);
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = Usage;
    bufferDesc.ByteWidth = ByteWidth;
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bufferDesc.CPUAccessFlags = CpuAccessFlags;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = Vertices;

    ID3D11Buffer* NewBuffer = nullptr;
    HRESULT hr = DXDevice->CreateBuffer(&bufferDesc, &initData, &NewBuffer);
    if (FAILED(hr))
        return hr;

    FVertexInfo VertexInfo;
    VertexInfo.NumVertices = static_cast<uint32>(ByteWidth / Stride);
    VertexInfo.VertexBuffer = NewBuffer;
    VertexInfo.Stride = Stride;
    VertexInfo.Offset = Offset;
    VertexBufferPool.Add(KeyName, VertexInfo);

    return S_OK;
}


template<typename T>
inline HRESULT FDXDBufferManager::CreateVertexBuffer(const FString& KeyName, const TArray<T>& vertices, D3D11_USAGE Usage, UINT CpuAccessFlags)
{
    FVertexInfo _temp;
    return CreateVertexBufferInternal(KeyName, vertices, _temp, Usage, CpuAccessFlags);
}


template<typename T>
HRESULT FDXDBufferManager::CreateDynamicVertexBuffer(const FString& KeyName, const TArray<T>& vertices, FVertexInfo& OutVertexInfo)
{
    return CreateVertexBufferInternal(KeyName, vertices, OutVertexInfo, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
}

template<typename T>
HRESULT FDXDBufferManager::CreateBufferGeneric(const FString& KeyName, T* data, UINT byteWidth, UINT bindFlags, D3D11_USAGE usage, UINT cpuAccessFlags)
{
    if (ConstantBufferPool.Contains(KeyName))
    {
        return S_OK;
    }
    
    byteWidth = Align16(byteWidth);

    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = byteWidth;
    desc.Usage = usage;
    desc.BindFlags = bindFlags;
    desc.CPUAccessFlags = cpuAccessFlags;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = data;

    ID3D11Buffer* buffer = nullptr;
    HRESULT hr = DXDevice->CreateBuffer(&desc, data ? &initData : nullptr, &buffer);
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Error Create Constant Buffer!"));
        return hr;
    }

    ConstantBufferPool.Add(KeyName, buffer);
    return S_OK;
}

template<typename T>
inline HRESULT FDXDBufferManager::CreateStructuredBufferGeneric(const FString& KeyName, T* Data, int32 NumElements, D3D11_USAGE Usage, UINT CpuAccessFlags)
{
    if (StructuredBufferPool.Contains(KeyName))
    {
        return S_OK;
    }

    D3D11_BUFFER_DESC Desc = {};
    Desc.ByteWidth = Align16(sizeof(T) * NumElements);
    Desc.Usage = Usage;
    Desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    Desc.CPUAccessFlags = CpuAccessFlags;
    Desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
    Desc.StructureByteStride = sizeof(T);

    D3D11_SUBRESOURCE_DATA InitData = {};
    InitData.pSysMem = Data;

    ID3D11Buffer* Buffer = nullptr;
    HRESULT Result = DXDevice->CreateBuffer(&Desc, Data ? &InitData : nullptr, &Buffer);
    if (FAILED(Result))
    {
        UE_LOG(ELogLevel::Error, TEXT("Error Create Structured Buffer!"));
        return Result;
    }

    D3D11_SHADER_RESOURCE_VIEW_DESC SrvDesc = {};
    SrvDesc.Format = DXGI_FORMAT_UNKNOWN;
    SrvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    SrvDesc.Buffer.FirstElement = 0;
    SrvDesc.Buffer.NumElements = NumElements;

    ID3D11ShaderResourceView* SRV = nullptr;
    Result = DXDevice->CreateShaderResourceView(Buffer, nullptr, &SRV);
    if (FAILED(Result))
    {
        UE_LOG(ELogLevel::Error, TEXT("Error Create Structured Buffer SRV!"));
        return Result;
    }

    StructuredBufferPool.Add(KeyName, Buffer);
    StructuredBufferSRVPool.Add(KeyName, SRV);
    StructuredBufferElementSizePool.Add(KeyName, NumElements);

    return S_OK;
}

template<typename T>
inline HRESULT FDXDBufferManager::CreateConstantBuffer(const FString& KeyName, UINT NumElements)
{
    if (NumElements == 0)
    {
        UE_LOG(ELogLevel::Error, TEXT("CreateConstantBuffer 호출: NumElements가 0입니다."));
        return E_FAIL;
    }
    if (ConstantBufferPool.Contains(KeyName))
    {
        return S_OK;
    }

    UINT byteWidth = Align16(sizeof(T) * NumElements);


    D3D11_BUFFER_DESC desc = {};
    desc.ByteWidth = byteWidth;
    desc.Usage = D3D11_USAGE_DYNAMIC;
    desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    ID3D11Buffer* buffer = nullptr;
    HRESULT hr = DXDevice->CreateBuffer(&desc, nullptr, &buffer);
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Error Create Constant Buffer!"));
        return hr;
    }

    ConstantBufferPool.Add(KeyName, buffer);
    return S_OK;
}

template<typename T>
void FDXDBufferManager::UpdateConstantBuffer(const FString& key, const T& data) const
{
    ID3D11Buffer* buffer = GetConstantBuffer(key);
    if (!buffer)
    {
        UE_LOG(ELogLevel::Error, TEXT("UpdateConstantBuffer 호출: 키 %s에 해당하는 buffer가 없습니다."), *key);
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = DXDeviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Buffer Map 실패, HRESULT: 0x%X"), hr);
        return;
    }
    auto a = sizeof(T);
    memcpy(mappedResource.pData, &data, sizeof(T));
    DXDeviceContext->Unmap(buffer, 0);
}

template<typename T>
void FDXDBufferManager::UpdateConstantBuffer(const FString& key, const TArray<T>& data) const
{
    ID3D11Buffer* buffer = GetConstantBuffer(key);
    if (!buffer)
    {
        UE_LOG(ELogLevel::Error, TEXT("UpdateConstantBuffer 호출: 키 %s에 해당하는 buffer가 없습니다."), *key);
        return;
    }

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = DXDeviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Buffer Map 실패, HRESULT: 0x%X"), hr);
        return;
    }

    memcpy(mappedResource.pData, data.GetData(), sizeof(T) * data.Num());
    DXDeviceContext->Unmap(buffer, 0);
}

template<typename T>
inline void FDXDBufferManager::UpdateStructuredBuffer(const FString& Key, const TArray<T>& Data) const
{
    ID3D11Buffer* Buffer = GetStructuredBuffer(Key);
    if (!Buffer)
    {
        UE_LOG(ELogLevel::Error, TEXT("UpdateConstantBuffer 호출: 키 %s에 해당하는 buffer가 없습니다."), *Key);
        return;
    }

    // 버퍼 생성할때 지정한 NumElements 만큼 최대 크기 제한
    int32 ElementSize = StructuredBufferElementSizePool[Key];
    ElementSize = FMath::Max(0, FMath::Min(ElementSize, Data.Num()));

    D3D11_MAPPED_SUBRESOURCE MappedResource;
    const HRESULT Result = DXDeviceContext->Map(Buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &MappedResource);
    if (FAILED(Result))
    {
        UE_LOG(ELogLevel::Error, TEXT("Buffer Map 실패, HRESULT: 0x%X"), Result);
        return;
    }
    memcpy(MappedResource.pData, Data.GetData(), sizeof(T) * ElementSize);
    DXDeviceContext->Unmap(Buffer, 0);
}

template<typename T>
void FDXDBufferManager::UpdateDynamicVertexBuffer(const FString& KeyName, const TArray<T>& vertices) const
{
    if (!VertexBufferPool.Contains(KeyName))
    {
        UE_LOG(ELogLevel::Error, TEXT("UpdateDynamicVertexBuffer 호출: 키 %s에 해당하는 버텍스 버퍼가 없습니다."), *KeyName);
        return;
    }
    FVertexInfo vbInfo = VertexBufferPool[KeyName];

    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = DXDeviceContext->Map(vbInfo.VertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("VertexBuffer Map 실패, HRESULT: 0x%X"), hr);
        return;
    }

    memcpy(mapped.pData, vertices.GetData(), sizeof(T) * vertices.Num());
    DXDeviceContext->Unmap(vbInfo.VertexBuffer, 0);
}

template<typename T>
void FDXDBufferManager::SafeRelease(T*& comObject)
{
    if (comObject)
    {
        comObject->Release();
        comObject = nullptr;
    }
}

