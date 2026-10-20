#include <wrl/client.h>

struct ID3D12Debug1;
struct ID3D12InfoQueue;
struct IDXGIDebug1;
struct IDXGIInfoQueue;
struct ID3D12Device;

namespace fluxion {
class DebugLayer {
public:
	static void Enable();
	static void Shutdown();
	static void InitializeForDevice(ID3D12Device* device);
	static void DumpLiveObjects();
	static void ClearStoredMessages();
private:
#if defined(_DEBUG)
	static inline Microsoft::WRL::ComPtr<ID3D12Debug1> m_d3d12Debug;
	static inline Microsoft::WRL::ComPtr<ID3D12InfoQueue> m_d3d12InfoQueue;
	static inline Microsoft::WRL::ComPtr<IDXGIDebug1> m_dxgiDebug;
#endif
};
}