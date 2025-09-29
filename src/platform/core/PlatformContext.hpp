#pragma once

#include <string>
#include <vector>

namespace Platform {
class UnixPlatformContext;
class WindowsPlatformContext;

class PlatformContext {
    friend class UnixPlatformContext;
    friend class WindowsPlatformContext;

   public:
    virtual ~PlatformContext() = default;

    virtual const std::vector<std::string>& arguments() const { return m_Arguments; }

    virtual const std::string& external_storage_directory() const { return m_ExtStorageDir; }

    // caching compiled shaders
    virtual const std::string& temp_directory() const { return m_TmpDir; }

   protected:
    std::vector<std::string> m_Arguments;
    std::string m_ExtStorageDir;
    std::string m_TmpDir;

    PlatformContext() = default;
};

}  // namespace Platform