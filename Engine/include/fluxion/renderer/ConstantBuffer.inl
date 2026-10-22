namespace fluxion {
template <typename T> requires(sizeof(T) % 256 == 0)
inline bool ConstantBuffer<T>::Initialize(ID3D12Device* device) {
	auto heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	auto resDesc = CD3DX12_RESOURCE_DESC::Buffer(sizeof(T));

	FLUXION_CHECK(device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_buffer))
	);
	FLUXION_CHECK(m_buffer->Map(0, nullptr, reinterpret_cast<void**>(&m_mappedData)));
	return true;
}

template <typename T> requires(sizeof(T) % 256 == 0)
inline T& ConstantBuffer<T>::GetData() {
	return *m_mappedData;
}

template <typename T> requires(sizeof(T) % 256 == 0)
inline D3D12_GPU_VIRTUAL_ADDRESS ConstantBuffer<T>::GetGPUVirtualAddress() const {
	return m_buffer->GetGPUVirtualAddress();
}
}