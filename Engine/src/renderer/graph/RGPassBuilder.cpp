#include <fluxion/renderer/graph/RGPassBuilder.hpp>
#include <fluxion/common/Common.hpp>

namespace fluxion {
void RGPassBuilder::Read(RGResource* res) {
	FLUXION_ENSURE(res);
	m_reads.push_back(res);
}

void RGPassBuilder::Write(RGResource* res) {
	FLUXION_ENSURE(res);
	m_writes.push_back(res);
}

void RGPassBuilder::ReadDepth(RGResource* res) {
	FLUXION_ENSURE(res);
	m_depthReads.push_back(res);
}

void RGPassBuilder::WriteDepth(RGResource* res) {
	FLUXION_ENSURE(res);
	m_depthWrites.push_back(res);
}

const std::vector<RGResource*>& RGPassBuilder::GetReads() const {
	return m_reads;
}

const std::vector<RGResource*>& RGPassBuilder::GetWrites() const {
	return m_writes;
}

const std::vector<RGResource*>& RGPassBuilder::GetDepthReads() const {
	return m_depthReads;
}

const std::vector<RGResource*>& RGPassBuilder::GetDepthWrites() const {
	return m_depthWrites;
}
}