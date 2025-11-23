#include "ResourceLocator.h"
#include <wx/stdpaths.h>
#include <wx/filename.h>

namespace {
    class StubResourceLocator : public IResourceLocator {
    public:
        StubResourceLocator() {
            wxFileName exe(wxStandardPaths::Get().GetExecutablePath());
            m_base = exe.GetPath().ToStdString();
        }
        std::string iconFile(const std::string& baseName) const override {
            // Platform-specific icon extension (T056)
#if defined(_WIN32)
            return m_base + "/" + baseName + ".ico";
#elif defined(__APPLE__)
            return m_base + "/" + baseName + ".icns";
#else
            // Linux and ChromeOS use .ico (T057)
            return m_base + "/" + baseName + ".ico";
#endif
        }
        std::string splashBitmap() const override { return m_base + "/Splash.bmp"; }
        std::string audioDir() const override { return m_base + "/audio"; }
        std::string ttsDir() const override { return m_base + "/tts/data"; }
        std::string translationsDir() const override { return m_base + "/lang/nl"; }
    private:
        std::string m_base;
    };
}

std::unique_ptr<IResourceLocator> CreateResourceLocator() {
    return std::unique_ptr<IResourceLocator>(new StubResourceLocator());
}
