#pragma once

#include <fluxion/renderer/graph/RGPass.hpp>

namespace fluxion {
class RenderGraph {
public:
	void AddPass(RGPass* pass);
	void Clear();
	void Execute(ID3D12GraphicsCommandList* cmdList);
private:
	void InsertBarriers(ID3D12GraphicsCommandList* cmdList, RGPassBuilder& builder);
private:
	std::vector<RGPass*> m_passes;
	std::vector<D3D12_RESOURCE_BARRIER> m_barrierScratch;
};
}