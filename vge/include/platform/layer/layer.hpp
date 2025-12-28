#pragma once

#include <string>
#include <string_view>

namespace Platform {

class Platform;

enum class Hook { ON_UPDATE, ON_START, ON_CLOSE, ON_ERROR, ON_PLATFORM_CLOSE, ON_UPDATE_UI };

class Layer {
   public:
    Layer(std::string_view name, std::string_view description);

    virtual ~Layer() = default;

    virtual std::vector<Hook> getHooks() = 0;

    virtual void onUpdate(float dt) {};
    virtual void onStart() {};
    virtual void onClose() {};
    virtual void onError() {};
    virtual void onPlatformClose() {};
    virtual void onUpdateUI() {};

    [[nodiscard]] const std::string_view getName() const;
    [[nodiscard]] const std::string_view getDescription() const;

    void setPlatform(Platform* platform);
    void clearPlatform();

   protected:
    Platform* m_Platform;

   private:
    std::string m_Name;
    std::string m_Description;
};

}  // namespace Platform
