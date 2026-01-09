#pragma once

#include <filesystem>
#include <unordered_map>

namespace Resource {
namespace fs = std::filesystem;

struct ResourceDescriptor {
    fs::path path;
    std::unordered_map<std::string, std::string> metadata;

    virtual ~ResourceDescriptor() = default;
};

struct TextureDescriptor : ResourceDescriptor {
    enum class Format { RGBA8, RGB8, SRGBA8, SRGB8 };
    enum class Filter { Linear, Nearest };
    enum class Tiling { Optimal, Linear };
    enum class SamplerAddressMode { Repeat, Clamp, Mirror };

    Format format = Format::SRGBA8;
    Filter minFilter = Filter::Linear;
    Filter magFilter = Filter::Linear;
    Tiling tiling = Tiling::Optimal;
    SamplerAddressMode addressMode = SamplerAddressMode::Repeat;
    bool generateMipmaps = true;
};

struct MeshDescriptor : ResourceDescriptor {
    bool calculateNormals = false;
    bool calculateTangents = false;
    float scale = 1.0f;
};

} // namespace Resource
