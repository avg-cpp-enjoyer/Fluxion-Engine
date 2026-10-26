#include <fluxion/renderer/graph/RenderGraph.hpp>
#include <fluxion/common/Common.hpp>
#include <directx/d3dx12.h>

namespace fluxion {
void RenderGraph::AddPass(RGPass* pass) {
	FLUXION_ENSURE(pass);
	m_passes.push_back(pass);
}

void RenderGraph::Clear() {
	m_passes.clear();
}

void RenderGraph::Execute(ID3D12GraphicsCommandList* cmdList) {
	for (auto* pass : m_passes) {
		RGPassBuilder builder;
		pass->Setup(builder);
		InsertBarriers(cmdList, builder);
		pass->Execute(cmdList);
	}
}

void RenderGraph::InsertBarriers(ID3D12GraphicsCommandList* cmdList, RGPassBuilder& builder) {
	m_barrierScratch.clear();

	auto addTransition = [&](RGResource* res, D3D12_RESOURCE_STATES targetState) -> void {
		if (res && res->GetState() != targetState) {
			m_barrierScratch.push_back(CD3DX12_RESOURCE_BARRIER::Transition(res->GetResource(), res->GetState(), targetState));
			res->SetState(targetState);
		}
	};

	for (auto* res : builder.GetReads()) {
		addTransition(res, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}
	for (auto* res : builder.GetWrites()) {
		addTransition(res, D3D12_RESOURCE_STATE_RENDER_TARGET);
	}

	if (!m_barrierScratch.empty()) {
		cmdList->ResourceBarrier(static_cast<uint32>(m_barrierScratch.size()), m_barrierScratch.data());
	}
}
}
