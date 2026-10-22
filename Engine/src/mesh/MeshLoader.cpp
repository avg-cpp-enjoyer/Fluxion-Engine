#include <fluxion/mesh/MeshLoader.hpp>
#include <fluxion/common/Common.hpp>
#include <fluxion/platform/win32/Win32Defs.hpp>

#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <DirectXMath.h>
#include <charconv>

namespace fluxion {
bool VertexKey::operator==(const VertexKey& other) const {
	static_assert(sizeof(VertexKey) == 48, "VertexKey size changed! Check memcmp safety.");
	return memcmp(this, &other, sizeof(VertexKey)) == 0;
}

usize VertexKeyHasher::operator()(const VertexKey& key) const {
	const uint32* keyPtr = reinterpret_cast<const uint32*>(&key);
	usize hash = 0;
	for (usize i = 0; i < sizeof(VertexKey) / sizeof(uint32); i++) {
		hash ^= std::hash<uint32>()(keyPtr[i]) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
	}

	return hash;
}

bool MeshLoader::LoadFromOBJ(const std::string& path, MeshData& outData) {
	std::ifstream file(path.c_str());
	FLUXION_ENSURE_RET(file.is_open(), false);

	Positions positions;
	UVs uvs;
	Normals normals;
	Vertices uniqueVertices;

	std::string line;
	while (std::getline(file, line)) {
		if (line.empty() || line[0] == '#') {
			continue;
		}

		std::istringstream iss(line);
		std::string type;
		iss >> type;

		if (type == "v") {
			HandleOBJV(positions, iss);
		} else if (type == "vt") {
			HandleOBJVT(uvs, iss);
		} else if (type == "vn") {
			HandleOBJVN(normals, iss);
		} else if (type == "f") {
			FaceParams params{ &outData, &uniqueVertices, &positions, &uvs, &normals };
			HandleOBJF(params, iss);
		}
	}

	OutputDebugStringA(("[Fluxion] Loaded " + std::to_string(outData.vertices.size()) + " vertices.\n").c_str());
	return true;
}

FaceIndices MeshLoader::ParseOBJFaceToken(const std::string_view token) {
	FaceIndices result{};
	usize slash1 = token.find('/');

	auto parseInt = [](std::string_view str) -> int32 {
		if (str.empty()) {
			return 0;
		}

		int32 value = 0;
		auto [ptr, errc] = std::from_chars(str.data(), str.data() + str.size(), value);
		return (errc == std::errc{}) ? value : 0;
	};

	if (slash1 == std::string_view::npos) {
		result.v = parseInt(token);
		return result;
	}

	result.v = parseInt(token.substr(0, slash1));
	usize slash2 = token.find('/', slash1 + 1);

	if (slash2 == std::string_view::npos) {
		result.vt = parseInt(token.substr(slash1 + 1));
		return result;
	}

	if (slash2 > slash1 + 1) {
		result.vt = parseInt(token.substr(slash1 + 1, slash2 - slash1 - 1));
	}

	result.vn = parseInt(token.substr(slash2 + 1));
	return result;
}

void MeshLoader::HandleOBJF(const FaceParams& params, std::istringstream& iss) {
	std::vector<uint32> faceIndices;
	std::string token;

	while (iss >> token) {
		FaceIndices indices = ParseOBJFaceToken(token);
		if (indices.v <= 0 || indices.v > static_cast<int32>(params.positions->size())) {
			continue;
		}

		VertexKey key{};
		key.pos = params.positions->at(indices.v - 1);

		if (indices.vt > 0 && indices.vt <= static_cast<int32>(params.uvs->size())) {
			key.uv = params.uvs->at(indices.vt - 1);
		}
		if (indices.vn > 0 && indices.vn <= static_cast<int32>(params.normals->size())) {
			key.normal = params.normals->at(indices.vn - 1);
		}

		auto it = params.uniqueVertices->find(key);
		if (it != params.uniqueVertices->end()) {
			faceIndices.push_back(it->second);
			continue;
		}

		uint32 newIndex = static_cast<uint32>(params.outData->vertices.size());
		params.outData->vertices.emplace_back(key.pos, key.normal, key.uv, DirectX::XMFLOAT4{ 1, 1, 1, 1 });
		params.uniqueVertices->emplace(key, newIndex);
		faceIndices.push_back(newIndex);
	}

	for (usize i = 1; i + 1 < faceIndices.size(); i++) {
		params.outData->indices.push_back(faceIndices[0]);
		params.outData->indices.push_back(faceIndices[i]);
		params.outData->indices.push_back(faceIndices[i + 1]);
	}
}

void MeshLoader::HandleOBJV(Positions& positions, std::istringstream& iss) {
	DirectX::XMFLOAT3 pos{};
	iss >> pos.x >> pos.y >> pos.z;
	positions.push_back(pos);
}

void MeshLoader::HandleOBJVT(UVs& uvs, std::istringstream& iss) {
	DirectX::XMFLOAT2 uv{};
	iss >> uv.x >> uv.y;
	uv.y = 1.0f - uv.y;
	uvs.push_back(uv);
}

void MeshLoader::HandleOBJVN(Normals& normals, std::istringstream& iss) {
	DirectX::XMFLOAT3 norm{};
	iss >> norm.x >> norm.y >> norm.z;
	normals.push_back(norm);
}
}
