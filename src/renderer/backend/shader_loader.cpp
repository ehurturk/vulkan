#include "shader_loader.hpp"
#include <fstream>
#include "core/assert.hpp"
#include "core/logger.hpp"

namespace Renderer {
std::vector<char> ShaderLoader::read_file(std::string_view filename) {
    LOG_DEBUG("Retrieved file: {}", filename);
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    ASSERT_MSG(file.is_open(), "Could not open shader file.");

    size_t fileSize = (size_t)file.tellg();
    std::vector<char> buf(fileSize);
    file.seekg(0);
    file.read(buf.data(), fileSize);
    file.close();

    return buf;
}
};  // namespace Renderer