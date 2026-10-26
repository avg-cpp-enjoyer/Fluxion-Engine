#pragma once

#include <fluxion/renderer/graph/RGResource.hpp>

#include <d3d12.h>
#include <vector>
#include <functional>
#include <string>
#include <memory>

namespace fluxion {
class RGPassBuilder {
public:
	void Read(RGResource* res);
	void Write(RGResource* res);
	const std::vector<RGResource*>& GetReads() const;
	const std::vector<RGResource*>& GetWrites() const;
private:
	std::vector<RGResource*> m_reads;
	std::vector<RGResource*> m_writes;
};
}