#include "shader_loader.hpp"
#include <fstream>
#include "core/assert.hpp"
#include "core/logger.hpp"

namespace Renderer {
std::vector<char> ShaderLoader::read_file(std::string_view filename) {
    CORE_LOG_INFO("Retrieved shader: {}", filename);
    std::ifstream file(filename, std::ios::ate | std::ios::binary);

    ASSERT_MSG(file.is_open(), "Could not open shader file.");

    const size_t fileSize = (size_t)file.tellg();
    std::vector<char> buf(fileSize);
    file.seekg(0);
    file.read(buf.data(), fileSize);
    file.close();

    return buf;
}
};  // namespace Renderer