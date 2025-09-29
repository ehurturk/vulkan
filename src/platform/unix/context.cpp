#include "context.hpp"

namespace Platform {

UnixPlatformContext::UnixPlatformContext(int argc, char** argv) : PlatformContext{} {
    m_Arguments.reserve(argc);

    for (int i = 0; i < argc; i++) {
        m_Arguments.emplace_back(argv[i]);
    }

    const char* tmpdir = std::getenv("TMPDIR");
    m_TmpDir = tmpdir ? std::string(tmpdir) + "/" : "/tmp/";
    m_ExtStorageDir = "";
}
}  // namespace Platform