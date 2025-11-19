#pragma once

#include <string_view>
#include <vector>
namespace Renderer {

class ShaderLoader {
   public:
    static std::vector<char> read_file(std::string_view filename);
};

};  // namespace Renderer