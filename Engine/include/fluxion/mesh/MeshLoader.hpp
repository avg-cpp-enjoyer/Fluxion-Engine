#pragma once

#include <fluxion/mesh/Vertex.hpp>
#include <fluxion/common/Common.hpp>

#include <string_view>
#include <unordered_map>
#include <string>

namespace fluxion {
namespace {
struct VertexKey {
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT4 color;
	bool operator==(const VertexKey& other) const;
};

struct VertexKeyHasher {
	usize operator()(const VertexKey& key) const;
};

struct FaceIndices {
	int32 v = 0;
	int32 vt = 0;
	int32 vn = 0;
};

using Positions = std::vector<DirectX::XMFLOAT3>;
using UVs = std::vector<DirectX::XMFLOAT2>;
using Vertices = std::unordered_map<VertexKey, uint32, VertexKeyHasher>;
using Normals = std::vector<DirectX::XMFLOAT3>;

struct FaceParams {
	MeshData* outData;
	Vertices* uniqueVertices;
	Positions* positions;
	UVs* uvs;
	Normals* normals;
};
}

class MeshLoader {
public:
	static bool LoadFromOBJ(const std::string& path, MeshData& outData);
private:
	static FaceIndices ParseOBJFaceToken(const std::string_view token);
	static void HandleOBJF(const FaceParams& params, std::istringstream& iss);
	static void HandleOBJV(Positions& positions, std::istringstream& iss);
	static void HandleOBJVT(UVs& uvs, std::istringstream& iss);
	static void HandleOBJVN(Normals& normals, std::istringstream& iss);
};
}