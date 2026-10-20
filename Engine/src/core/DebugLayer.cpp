#include <fluxion/core/DebugLayer.hpp>
#include <fluxion/common/Common.hpp>

#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>

namespace fluxion {
void DebugLayer::Enable() {
#if defined(_DEBUG)
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&m_d3d12Debug)))) {
		m_d3d12Debug->EnableDebugLayer();
		m_d3d12Debug->SetEnableGPUBasedValidation(TRUE);
		m_d3d12Debug->SetEnableSynchronizedCommandQueueValidation(TRUE);
	}

	if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&m_dxgiDebug)))) {
		m_dxgiDebug->EnableLeakTrackingForThread();
	}
#endif
}

void DebugLayer::Shutdown() {
#if defined(_DEBUG)
	m_d3d12InfoQueue.Reset();
	m_d3d12Debug.Reset();
	m_dxgiDebug.Reset();
#endif
}

void DebugLayer::InitializeForDevice(ID3D12Device* device) {
#if defined(_DEBUG)
	FLUXION_ENSURE(device);

	if (SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&m_d3d12InfoQueue)))) {
		m_d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
		m_d3d12InfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
	}
#endif

}

void DebugLayer::DumpLiveObjects() {
#if defined(_DEBUG)
	FLUXION_SAFE_INVOKE(m_dxgiDebug, ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL));
	FLUXION_ENSURE(m_d3d12InfoQueue);

	OutputDebugStringA("[Fluxion] D3D12 InfoQueue messages:\n");
	uint64 storedMessages = m_d3d12InfoQueue->GetNumStoredMessages();

	for (uint64 i = 0; i < storedMessages; i++) {
		usize len = 0;
		m_d3d12InfoQueue->GetMessage(i, nullptr, &len);

		auto* msg = static_cast<D3D12_MESSAGE*>(::operator new(len));
		if (msg && SUCCEEDED(m_d3d12InfoQueue->GetMessage(i, msg, &len))) {
			OutputDebugStringA(msg->pDescription);
			OutputDebugStringA("\n");
		}

		::operator delete(msg);
	}
#endif
}

void DebugLayer::ClearStoredMessages() {
#if defined(_DEBUG)
	FLUXION_ENSURE(m_d3d12InfoQueue);
	m_d3d12InfoQueue->ClearStoredMessages();
#endif
}
}