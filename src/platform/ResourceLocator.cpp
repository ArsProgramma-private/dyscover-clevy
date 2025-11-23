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
        std::string iconFile(const std::string& baseName) const override { return m_base + "/" + baseName; }
        std::string splashBitmap() const override { return m_base + "/Splash"; }
        std::string audioDir() const override { return m_base + "/audio"; }
        std::string ttsDir() const override { return m_base + "/tts"; }
        std::string translationsDir() const override { return m_base + "/lang"; }
    private:
        std::string m_base;
    };
}

std::unique_ptr<IResourceLocator> CreateResourceLocator() {
    return std::unique_ptr<IResourceLocator>(new StubResourceLocator());
}
