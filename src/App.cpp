//
// App.cpp
//

#include <wx/msgdlg.h>
#include <wx/splash.h>

#include "App.h"
#include "Config.h"
#include "Core.h"
#include "PreferencesDialog.h"
#include "ResourceLoader.h"
#include "TrayIcon.h"
#include "layouts/LayoutRegistry.h"

bool App::OnInit()
{
#if defined __LANGUAGE_NL__
    m_pLocale = new wxLocale(wxLANGUAGE_DUTCH);
#elif defined __LANGUAGE_NL_BE__
    m_pLocale = new wxLocale(wxLANGUAGE_DUTCH_BELGIAN);
#else
#error Unsupported language.
#endif

    wxTranslations* pTranslations = wxTranslations::Get();
#ifdef __WINDOWS__
    pTranslations->SetLoader(new wxResourceTranslationsLoader());
#else
    pTranslations->SetLoader(new wxFileTranslationsLoader());
#endif
    pTranslations->AddCatalog("Dyscover");

    m_pSingleInstanceChecker = new wxSingleInstanceChecker();
    if (m_pSingleInstanceChecker->IsAnotherRunning())
    {
        wxMessageBox(_("Another instance is already running."), _("Clevy Dyscover"), wxSTAY_ON_TOP);

        // Clean up now because OnExit() won't be called since we are returning false here.
        delete m_pSingleInstanceChecker;
        delete m_pLocale;

        return false;
    }

    m_pConfig = new Config();

        // Restore runtime-selected layout at startup based on persisted config
        // Default to language-specific classic if unknown.
        {
        Layout layout = m_pConfig->GetLayout();
    #if defined(__LANGUAGE_NL__)
        const char* name = (layout == Layout::Default) ? "dutchdefault"
                     : (layout == Layout::Classic) ? "dutchclassic"
    #ifdef __LANGUAGE_NL__
                     : (layout == Layout::KWeC) ? "dutchkwec"
    #endif
                     : "dutchclassic";
        Dyscover::LayoutRegistry::Instance().SetActiveLayout(name);
    #elif defined(__LANGUAGE_NL_BE__)
        const char* name = (layout == Layout::Default) ? "flemishdefault"
                     : /* Layout::Classic */ "flemishclassic";
        Dyscover::LayoutRegistry::Instance().SetActiveLayout(name);
    #endif
        }

#ifdef __LICENSING_DEMO__
    m_pDemoLicensing = new DemoLicensing(m_pConfig, this);
    if (m_pDemoLicensing->HasDemoLicenseExpired())
    {
        wxMessageBox(_("Your 30-day demo license has expired! Please purchase a license."), _("Clevy Dyscover demo license expired"), wxSTAY_ON_TOP);

        // Clean up now because OnExit() won't be called since we are returning false here.
        delete m_pDemoLicensing;
        delete m_pConfig;
        delete m_pSingleInstanceChecker;
        delete m_pLocale;

        return false;
    }
#endif

    // No need to save pointer here because wxSplashScreen will automatically delete itself.
    new wxSplashScreen(LoadSplashBitmap(), wxSPLASH_CENTRE_ON_SCREEN | wxSPLASH_TIMEOUT, 3000, nullptr, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxSTAY_ON_TOP);

    // Initialize pointers to nullptr before attempting creation to ensure safe cleanup on failure
    m_pDevice = nullptr;
    m_pCore = nullptr;
    m_pPreferencesDialog = nullptr;
    m_pTrayIcon = nullptr;

    try {
        m_pDevice = Device::Create(this);
        m_pCore = new Core(this, m_pConfig, m_pDevice);
        m_pPreferencesDialog = new PreferencesDialog(this, m_pConfig);
        m_pTrayIcon = new TrayIcon(this, m_pConfig);
    } catch (const std::exception& e) {
        wxMessageBox(wxString::Format(_("Failed to initialize application: %s"), e.what()), _("Clevy Dyscover"), wxSTAY_ON_TOP);
        return false;
    } catch (...) {
        wxMessageBox(_("Failed to initialize application due to an unknown error."), _("Clevy Dyscover"), wxSTAY_ON_TOP);
        return false;
    }

    return true;
}

int App::OnExit()
{
    // Safe cleanup with nullptr checks
    if (m_pTrayIcon) delete m_pTrayIcon;
    if (m_pPreferencesDialog) delete m_pPreferencesDialog;
    if (m_pDevice) delete m_pDevice;
#ifdef __LICENSING_DEMO__
    if (m_pDemoLicensing) delete m_pDemoLicensing;
#endif
    if (m_pCore) delete m_pCore;
    if (m_pConfig) delete m_pConfig;

    if (m_pSingleInstanceChecker) delete m_pSingleInstanceChecker;
    if (m_pLocale) delete m_pLocale;

    return wxApp::OnExit();
}

void App::OnClevyKeyboardConnected()
{
    // Note: Core receives device events directly from DeviceDetector and will
    // call UpdatePreferencesDialog/UpdateTrayIcon as needed. This callback is
    // from the deprecated Device class and should not forward to Core to avoid
    // duplicate event handling.
}

void App::OnClevyKeyboardDisconnected()
{
    // Note: Core receives device events directly from DeviceDetector and will
    // call UpdatePreferencesDialog/UpdateTrayIcon as needed. This callback is
    // from the deprecated Device class and should not forward to Core to avoid
    // duplicate event handling.
}

void App::OnDemoTimeLimitExpired()
{
    wxMessageBox(_("Your 30 minute demo has expired. Please purchase a license."), _("Clevy Dyscover demo license"), wxSTAY_ON_TOP);

    Exit();
}

void App::ShowPreferencesDialog()
{
    m_pPreferencesDialog->Show();
}

void App::UpdatePreferencesDialog()
{
    if (m_pPreferencesDialog)
    {
        m_pPreferencesDialog->TransferDataToWindow();
    }
}

void App::UpdateTrayIcon()
{
    if (m_pTrayIcon)
    {
        m_pTrayIcon->UpdateIcon();
    }
}

bool App::IsClevyKeyboardPresent()
{
    return m_pCore ? m_pCore->IsKeyboardConnected() : false;
}

#ifdef __LICENSING_DEMO__
int App::GetDemoDaysRemaining()
{
    return m_pDemoLicensing->GetDaysRemaining();
}
#endif

wxIMPLEMENT_APP(App);
