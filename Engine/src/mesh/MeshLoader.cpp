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

	CalculateTangents(outData);
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

void MeshLoader::CalculateTangents(MeshData& outData) {
	std::vector<DirectX::XMFLOAT3> tan1(outData.vertices.size(), { 0.0f, 0.0f, 0.0f });
	std::vector<DirectX::XMFLOAT3> tan2(outData.vertices.size(), { 0.0f, 0.0f, 0.0f });

	for (usize i = 0; i < outData.indices.size(); i += 3) {
		uint32 i0 = outData.indices[i];
		uint32 i1 = outData.indices[i + 1];
		uint32 i2 = outData.indices[i + 2];

		const auto& v0 = outData.vertices[i0];
		const auto& v1 = outData.vertices[i1];
		const auto& v2 = outData.vertices[i2];

		DirectX::XMVECTOR p0 = v0.PositionVec();
		DirectX::XMVECTOR p1 = v1.PositionVec();
		DirectX::XMVECTOR p2 = v2.PositionVec();

		DirectX::XMVECTOR uv0 = v0.UVVec();
		DirectX::XMVECTOR uv1 = v1.UVVec();
		DirectX::XMVECTOR uv2 = v2.UVVec();

		DirectX::XMVECTOR e1 = DirectX::XMVectorSubtract(p1, p0);
		DirectX::XMVECTOR e2 = DirectX::XMVectorSubtract(p2, p0);

		float32 duv1x = DirectX::XMVectorGetX(uv1) - DirectX::XMVectorGetX(uv0);
		float32 duv1y = DirectX::XMVectorGetY(uv1) - DirectX::XMVectorGetY(uv0);
		float32 duv2x = DirectX::XMVectorGetX(uv2) - DirectX::XMVectorGetX(uv0);
		float32 duv2y = DirectX::XMVectorGetY(uv2) - DirectX::XMVectorGetY(uv0);
		float32 det = duv1x * duv2y - duv2x * duv1y;
		
		if (std::abs(det) < 1e-6f) continue;
		float32 scaleFactor = 1.0f / det;

		DirectX::XMVECTOR e1_scaled_duv2y = DirectX::XMVectorScale(e1, duv2y);
		DirectX::XMVECTOR e2_scaled_duv1y = DirectX::XMVectorScale(e2, duv1y);
		DirectX::XMVECTOR e2_scaled_duv1x = DirectX::XMVectorScale(e2, duv1x);
		DirectX::XMVECTOR e1_scaled_duv2x = DirectX::XMVectorScale(e1, duv2x);

		DirectX::XMVECTOR tangentSum = DirectX::XMVectorSubtract(e1_scaled_duv2y, e2_scaled_duv1y);
		DirectX::XMVECTOR bitangentSum = DirectX::XMVectorSubtract(e2_scaled_duv1x, e1_scaled_duv2x);

		DirectX::XMVECTOR tangent = DirectX::XMVectorScale(tangentSum, scaleFactor);
		DirectX::XMVECTOR bitangent = DirectX::XMVectorScale(bitangentSum, scaleFactor);

		DirectX::XMFLOAT3 t3, b3;
		DirectX::XMStoreFloat3(&t3, tangent);
		DirectX::XMStoreFloat3(&b3, bitangent);

		auto Add = [](DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) -> void {
			a.x += b.x; a.y += b.y; a.z += b.z;
		};

		Add(tan1[i0], t3); Add(tan1[i1], t3); Add(tan1[i2], t3);
		Add(tan2[i0], b3); Add(tan2[i1], b3); Add(tan2[i2], b3);
	}

	for (size_t i = 0; i < outData.vertices.size(); i++) {
		auto& v = outData.vertices[i];

		DirectX::XMVECTOR n = v.NormalVec();
		DirectX::XMVECTOR t = DirectX::XMLoadFloat3(&tan1[i]);

		if (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(t)) < 1e-6f) {
			float32 nYAbs = std::abs(DirectX::XMVectorGetY(n));
			DirectX::XMVECTOR upY = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
			DirectX::XMVECTOR upX = DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f);
			DirectX::XMVECTOR up = (nYAbs < 0.99f) ? upY : upX;
			t = DirectX::XMVector3Cross(up, n);
		}

		float dotNT = DirectX::XMVectorGetX(DirectX::XMVector3Dot(n, t));
		t = DirectX::XMVectorSubtract(t, DirectX::XMVectorScale(n, dotNT));
		t = DirectX::XMVector3Normalize(t);

		DirectX::XMVECTOR bCalc = DirectX::XMVector3Cross(n, t);
		DirectX::XMVECTOR bAccum = DirectX::XMLoadFloat3(&tan2[i]);

		float32 sign = (DirectX::XMVectorGetX(DirectX::XMVector3Dot(bCalc, bAccum)) < 0.0f) ? -1.0f : 1.0f;
		v.SetTangent(DirectX::XMVectorSetW(t, sign));
	}
}
}
