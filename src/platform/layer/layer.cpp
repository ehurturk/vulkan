#include "layer.hpp"

#include "core/assert.hpp"

namespace Platform {

Layer::Layer(std::string_view name, std::string_view description)
    : m_Platform(nullptr), m_Name(name), m_Description(description) {}

const std::string_view Layer::getName() const {
    return m_Name;
}

const std::string_view Layer::getDescription() const {
    return m_Description;
}

void Layer::setPlatform(Platform* platform) {
    ASSERT_MSG(m_Platform == nullptr, "[Layer]:Platform is already set!")
    m_Platform = platform;
}

void Layer::clearPlatform() {
    m_Platform = nullptr;
}

}  // namespace Platform
