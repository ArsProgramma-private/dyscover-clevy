//
// ResourceLoader.cpp
//

#include <wx/filename.h>
#include <wx/stdpaths.h>

#include "ResourceLoader.h"
#include "platform/ResourceLocator.h"

static std::unique_ptr<IResourceLocator> g_locator;

static IResourceLocator* GetLocator() {
    if (!g_locator) {
        g_locator = CreateResourceLocator();
    }
    return g_locator.get();
}

wxIcon LoadIcon(const wxString& name)
{
    std::string iconPath = GetLocator()->iconFile(name.ToStdString());
#ifdef __WINDOWS__
    return wxIcon(name, wxBITMAP_TYPE_ICO_RESOURCE);
#else
    return wxIcon(wxString::FromUTF8(iconPath), 
#ifdef __APPLE__
                  wxBITMAP_TYPE_ICON
#else
                  wxBITMAP_TYPE_ICO
#endif
    );
#endif
}

wxIcon LoadDyscoverIcon()
{
    return LoadIcon("Icon");
}

wxIconArray LoadDyscoverIcons()
{
    wxIconArray icons;

    for (int i = 0; i <= 6; i++)
    {
        icons.Add(LoadIcon(wxString::Format("Icon%d", i)));
    }

    return icons;
}

wxBitmap LoadSplashBitmap()
{
#ifdef __WINDOWS__
    return wxBitmap("Splash", wxBITMAP_TYPE_BMP_RESOURCE);
#else
    std::string splashPath = GetLocator()->splashBitmap();
    return wxBitmap(wxString::FromUTF8(splashPath), wxBITMAP_TYPE_BMP);
#endif
}

wxString GetExecutablePath()
{
    wxFileName filename(wxStandardPaths::Get().GetExecutablePath());
    return filename.GetPath();
}

wxString GetSoundFilesPath()
{
    return wxString::FromUTF8(GetLocator()->audioDir());
}

wxString GetTTSDataPath()
{
    return wxString::FromUTF8(GetLocator()->ttsDir());
}

wxString GetTranslationsPath()
{
    return wxString::FromUTF8(GetLocator()->translationsDir());
}
