#pragma once
#include <memory>
#include <string>

class IResourceLocator {
public:
    virtual std::string iconFile(const std::string& baseName) const = 0;
    virtual std::string splashBitmap() const = 0;
    virtual std::string audioDir() const = 0;
    virtual std::string ttsDir() const = 0;
    virtual std::string translationsDir() const = 0;
    virtual ~IResourceLocator() = default;
};

std::unique_ptr<IResourceLocator> CreateResourceLocator();
