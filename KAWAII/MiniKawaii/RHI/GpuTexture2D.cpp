#include "../pch.h"
#include "GpuTexture2D.h"
#include "RenderDevice.h"
#include "GpuResourceDescriptor.h"
#include "CommandContext.h"

namespace RHI
{
	GpuTexture2D::GpuTexture2D(UINT32 width, UINT32 height, DXGI_FORMAT format, UINT64 rowPitchBytes, const void* initialData)
		: GpuTexture(width, height, D3D12_RESOURCE_DIMENSION_TEXTURE2D, format)
	{
		m_UsageState = D3D12_RESOURCE_STATE_COPY_DEST;

		D3D12_RESOURCE_DESC texDesc{};
		texDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		texDesc.Width = width;
		texDesc.Height = (UINT)height;
		texDesc.DepthOrArraySize = 1;
		texDesc.MipLevels = 1;
		texDesc.Format = format;
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		texDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_HEAP_PROPERTIES heapProp{};
		heapProp.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProp.CreationNodeMask = 1;
		heapProp.VisibleNodeMask = 1;

		ThrowIfFailed(RenderDevice::GetSingleton().GetD3D12Device()->CreateCommittedResource(&heapProp, D3D12_HEAP_FLAG_NONE, &texDesc,
			m_UsageState, nullptr, IID_PPV_ARGS(&m_pResource)));

		D3D12_SUBRESOURCE_DATA texResourceData;
		texResourceData.pData = initialData;
		texResourceData.RowPitch = rowPitchBytes;
		texResourceData.SlicePitch = rowPitchBytes * height;

		CommandContext::InitializeTexture(*this, 1, &texResourceData);
	}

	std::shared_ptr<GpuResourceDescriptor> GpuTexture2D::CreateSRV()
	{
		std::shared_ptr<GpuResourceDescriptor> descriptor = std::make_shared<GpuResourceDescriptor>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, shared_from_this());
		
		D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
		SRVDesc.Format = m_format;
		SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		SRVDesc.Texture2D.MostDetailedMip = 0;
		SRVDesc.Texture2D.MipLevels = 1;

		RenderDevice::GetSingleton().GetD3D12Device()->CreateShaderResourceView(m_pResource.Get(), &SRVDesc, descriptor->GetCpuHandle());

		return descriptor;
	}
}