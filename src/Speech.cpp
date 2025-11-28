//
// Speech.cpp
//

#include <thread>

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <wx/log.h>
#include <wx/dir.h>
#include <wx/arrstr.h>
#include <wx/filename.h>
#include "Audio.h"
#include "Speech.h"

#ifdef  __BORLANDC__
#pragma package(smart_init)
#endif

static const int kChannels = 1;
static const int kSampleRate = 22050;
static const int kSampleSize = 2;

static const char kLicense[] = "<?xml version=\"1.0\"?>\
<license version=\"1.0\" licid=\"e73db530bb7c651ea041eca0eb7faba2\" subject=\"rSpeak SDK SuperLicense\">\
	<issued>2020-09-23</issued>\
	<licensee>\
		<company name=\"BNC Distribution\" address=\"Wasaweg 3a, 9723JD Groningen, The Netherlands\"/>\
		<contact name=\"Bertran van den Hoff\" email=\"bertran.vandenhoff@bnc-distribution.nl\"/>\
	</licensee>\
	<product name=\"SDK rSpeak\"/>\
	<general channels=\"0\" speed=\"5\" textlimit=\"0\" expires=\"-1\"/>\
	<voice name=\"Ilse\" vendor=\"rSpeak\"/>\
	<voice name=\"Max\" vendor=\"rSpeak\"/>\
	<voice name=\"Gina\" vendor=\"rSpeak\"/>\
	<voice name=\"Veerle\" vendor=\"rSpeak\"/>\
	<signature id=\"6711250ff810fa1b50fa806f607ecbd66fee09195d42b52727938855daf579f761ae0acad579ffd5fb694c281b2a3665196abf8e1a74d08cfd8474bcd96cd2ec\"/>\
</license>";

#ifndef __NO_TTS__
Speech::Speech() : m_rstts(nullptr), m_quit(false) {}
Speech::~Speech() { Term(); }

bool Speech::Init(const char* basedir, const char* lang, const char* voice)
{
    wxLogInfo("[Speech] Init: baseDir='%s', lang='%s', voice='%s'", basedir ? basedir : "(null)", lang ? lang : "(null)", voice ? voice : "(null)");
    m_rstts = rsttsInit(basedir);
    if (m_rstts == nullptr) {
        wxLogWarning("[Speech] rsttsInit failed (baseDir='%s')", basedir ? basedir : "(null)");
        return false;
    }

    // Validate required TTS data files exist before configuring language/voice
    // Engine expects base dir containing data/ subdirectory with actual files
    if (basedir && lang && voice) {
        wxString baseStr = wxString::FromUTF8(basedir);
        wxString dataDir = baseStr + "/data";
        
        struct Req { const char* kind; wxString name; bool optional; };
        Req required[] = {
            { "lang", wxString::Format("%s.db", lang), false },
            { "lang", wxString::Format("%s.fsa", lang), false },
            { "lang", wxString::Format("%s.fst", lang), false },
            { "voice", wxString::Format("%s.db", voice), false },
            { "voice", wxString::Format("%s.fon", voice), false },
            { "voice", wxString::Format("%s.udb", voice), false },
            { "voice", wxString::Format("%s.opu", voice), false },
        };
        int missing = 0;
        for (const auto& r : required) {
            wxFileName fn(dataDir, r.name);
            if (!fn.FileExists()) {
                if (!r.optional) {
                    wxLogWarning("[Speech] Missing %s file: '%s' in '%s'", r.kind, r.name.c_str(), dataDir.c_str());
                    missing++;
                } else {
                    wxLogInfo("[Speech] Optional file not found: '%s' (ok)", r.name.c_str());
                }
            }
        }
        if (missing > 0) {
            wxLogWarning("[Speech] Required TTS files missing in '%s' (%d file(s)) — aborting TTS init", dataDir.c_str(), missing);
            Term();
            return false;
        }
        wxLogInfo("[Speech] All required TTS files validated in '%s'", dataDir.c_str());
    }

    int result = rsttsSetParameter(m_rstts, RSTTS_PARAM_LICENSE_BUFFER, RSTTS_TYPE_STRING, kLicense);
    if (RSTTS_ERROR(result)) { wxLogWarning("[Speech] rsttsSetParameter(LICENSE) failed (code=%d)", result); Term(); return false; }

    const int responsiveness = RSTTS_RESPONSIVENESS_FAST;
    result = rsttsSetParameter(m_rstts, RSTTS_PARAM_RESPONSIVENESS_SETTING, RSTTS_TYPE_INT, &responsiveness);
    if (RSTTS_ERROR(result)) { wxLogWarning("[Speech] rsttsSetParameter(RESPONSIVENESS) failed (code=%d)", result); Term(); return false; }

    result = rsttsSetSampleRate(m_rstts, kSampleRate);
    if (RSTTS_ERROR(result)) { wxLogWarning("[Speech] rsttsSetSampleRate(%d) failed (code=%d)", kSampleRate, result); Term(); return false; }

    // Explicitly set dictionary and audio directories to avoid implicit path issues
    if (basedir) {
        wxString dataPath = wxString::FromUTF8(basedir) + "/data";
        wxCharBuffer dataUtf8 = dataPath.utf8_str();
        const char* dataC = dataUtf8.data();
        int rdd = rsttsSetDictsdir(m_rstts, dataC);
        if (RSTTS_ERROR(rdd)) { wxLogWarning("[Speech] rsttsSetDictsdir('%s') failed (code=%d)", dataC, rdd); }
        int rad = rsttsSetAudiodir(m_rstts, dataC);
        if (RSTTS_ERROR(rad)) { wxLogWarning("[Speech] rsttsSetAudiodir('%s') failed (code=%d)", dataC, rad); }
    }

    // Open audio before setting language/voice (some engines require audio system ready)
    if (!m_audio.Open(kChannels, kSampleRate, paInt16)) { 
        wxLogWarning("[Speech] Audio::Open failed (channels=%d, rate=%d)", kChannels, kSampleRate); 
        Term(); 
        return false; 
    }
    wxLogInfo("[Speech] Audio opened successfully");

    result = rsttsSetAudioCallback(m_rstts, TTSAudioCallback, this);
    if (RSTTS_ERROR(result)) { 
        wxLogWarning("[Speech] rsttsSetAudioCallback failed (code=%d)", result); 
        Term(); 
        return false; 
    }
    wxLogInfo("[Speech] Audio callback set");

    // Log what engine sees in data directory for diagnostics
    if (basedir) {
        wxString dataPath = wxString::FromUTF8(basedir) + "/data";
        wxLogInfo("[Speech] Enumerating TTS data directory: %s", dataPath.c_str());
        wxDir dataDir(dataPath);
        if (dataDir.IsOpened()) {
            wxString filename;
            bool cont = dataDir.GetFirst(&filename);
            while (cont) {
                wxLogInfo("[Speech]   Found: %s", filename.c_str());
                cont = dataDir.GetNext(&filename);
            }
        }
    }

    // Try original language first (e.g., nl_nl), then fall back to base (nl)
    wxLogInfo("[Speech] Attempting language: '%s'", lang ? lang : "(null)");
    result = rsttsSetLanguage(m_rstts, lang);
    if (RSTTS_ERROR(result)) {
        wxLogWarning("[Speech] rsttsSetLanguage('%s') failed (code=%d) - attempting fallbacks", lang ? lang : "(null)", result);
        // Enumerate directory contents for diagnostics
        if (basedir) {
            wxString dir = wxString::FromUTF8(basedir);
            wxDir d(dir);
            if (d.IsOpened()) {
                wxString filename;
                bool cont = d.GetFirst(&filename, wxEmptyString, wxDIR_FILES);
                int count = 0;
                while (cont) {
                    if (count == 0) {
                        wxLogInfo("[Speech] Listing files in '%s'", dir.c_str());
                    }
                    if (count < 25) {
                        wxLogInfo("[Speech]   - %s", filename.c_str());
                    } else if (count == 25) {
                        wxLogInfo("[Speech]   ... (truncated list)");
                    }
                    ++count;
                    cont = d.GetNext(&filename);
                }
                wxLogInfo("[Speech] Directory '%s' contains %d files (listed %d)", dir.c_str(), count, count < 25 ? count : 25);
            } else {
                wxLogWarning("[Speech] Could not open directory '%s' for listing", dir.c_str());
            }
        }

        // Try alternative language codes if original is locale form
        const char* fallbacks[] = { "nl", "nl-NL", "NL", nullptr };
        bool langOk = false;
        for (int i = 0; fallbacks[i] != nullptr && !langOk; ++i) {
            if (lang && strcmp(lang, fallbacks[i]) == 0) continue; // skip identical
            int r2 = rsttsSetLanguage(m_rstts, fallbacks[i]);
            if (RSTTS_SUCCESS(r2)) {
                wxLogInfo("[Speech] Fallback language '%s' succeeded", fallbacks[i]);
                langOk = true;
                break;
            } else {
                wxLogWarning("[Speech] Fallback rsttsSetLanguage('%s') failed (code=%d)", fallbacks[i], r2);
            }
        }

        // If still not ok and path ends with /data, try parent directory re-init
        if (!langOk && basedir) {
            std::string baseStr(basedir);
            if (baseStr.size() >= 5 && baseStr.rfind("data") == baseStr.size() - 4) {
                wxLogWarning("[Speech] Retrying init with parent directory of 'data' (engine may expect root containing 'data/')");
                Term();
                // Derive parent: remove trailing /data or \data
                size_t pos = baseStr.find_last_of("/\\");
                if (pos != std::string::npos) {
                    std::string parent = baseStr.substr(0, pos); // remove /data
                    m_rstts = rsttsInit(parent.c_str());
                    if (m_rstts) {
                        wxLogInfo("[Speech] Re-init succeeded with baseDir='%s'", parent.c_str());
                        // Reapply parameters (sample rate etc.)
                        int rL = rsttsSetParameter(m_rstts, RSTTS_PARAM_LICENSE_BUFFER, RSTTS_TYPE_STRING, kLicense);
                        if (RSTTS_ERROR(rL)) { wxLogWarning("[Speech] License set failed after re-init (code=%d)", rL); Term(); return false; }
                        int rR = rsttsSetParameter(m_rstts, RSTTS_PARAM_RESPONSIVENESS_SETTING, RSTTS_TYPE_INT, &responsiveness);
                        if (RSTTS_ERROR(rR)) { wxLogWarning("[Speech] Resp set failed after re-init (code=%d)", rR); Term(); return false; }
                        int rSR = rsttsSetSampleRate(m_rstts, kSampleRate);
                        if (RSTTS_ERROR(rSR)) { wxLogWarning("[Speech] SampleRate set failed after re-init (code=%d)", rSR); Term(); return false; }
                        // Try original language again then fallbacks
                        int rLang = rsttsSetLanguage(m_rstts, lang);
                        if (RSTTS_SUCCESS(rLang)) {
                            wxLogInfo("[Speech] Language '%s' succeeded after parent re-init", lang);
                            langOk = true;
                        } else {
                            wxLogWarning("[Speech] Language '%s' still failed after parent re-init (code=%d)", lang, rLang);
                            for (int i = 0; fallbacks[i] != nullptr && !langOk; ++i) {
                                int r3 = rsttsSetLanguage(m_rstts, fallbacks[i]);
                                if (RSTTS_SUCCESS(r3)) { wxLogInfo("[Speech] Fallback '%s' succeeded after parent re-init", fallbacks[i]); langOk = true; break; }
                                else { wxLogWarning("[Speech] Fallback '%s' failed after parent re-init (code=%d)", fallbacks[i], r3); }
                            }
                        }
                    } else {
                        wxLogWarning("[Speech] Re-init with parent directory failed (parent='%s')", parent.c_str());
                    }
                }
            }
        }

        if (!langOk) { wxLogWarning("[Speech] All language attempts failed; aborting TTS init"); Term(); return false; }
    } else {
        wxLogInfo("[Speech] Language '%s' set successfully", lang ? lang : "(null)");
    }

    // Try setting voice - error -202 often means voice not in license or incompatible format
    // First attempt: use provided voice name
    result = rsttsSetVoiceByName(m_rstts, voice);
    if (RSTTS_ERROR(result)) {
        const char* errMsg = rsttsGetErrorMessage(m_rstts);
        wxLogWarning("[Speech] rsttsSetVoiceByName('%s') failed (code=%d, msg='%s')", voice ? voice : "(null)", result, errMsg ? errMsg : "(none)");
        // Attempt gender-based fallback(s)
        const char* genders[] = { "female", "neutral", nullptr };
        bool voiceOk = false;
        for (int gi = 0; genders[gi] != nullptr && !voiceOk; ++gi) {
            int rg = rsttsSetVoiceByGender(m_rstts, genders[gi]);
            if (RSTTS_SUCCESS(rg)) {
                wxLogInfo("[Speech] Fallback rsttsSetVoiceByGender('%s') succeeded", genders[gi]);
                voiceOk = true;
                break;
            } else {
                const char* gmsg = rsttsGetErrorMessage(m_rstts);
                wxLogWarning("[Speech] Fallback rsttsSetVoiceByGender('%s') failed (code=%d, msg='%s')", genders[gi], rg, gmsg ? gmsg : "(none)");
            }
        }
        if (!voiceOk) {
            wxLogInfo("[Speech] Continuing without explicit voice (engine will auto-select default for language)");
        }
    } else {
        wxLogInfo("[Speech] Voice '%s' set successfully", voice ? voice : "(null)");
    }

    // Log language/voice data version numbers if available
    unsigned long langv = 0, voicev = 0;
    int verRes = rsttsGetLanguageVoiceVersion(m_rstts, nullptr, nullptr, &langv, &voicev);
    if (RSTTS_SUCCESS(verRes)) {
        wxLogInfo("[Speech] Language/Voice version: lang=%lu (major=%lu minor=%lu) voice=%lu (major=%lu minor=%lu)",
                  langv,
                  (unsigned long)RSTTS_LANGVOICEVER_MAJOR(langv), (unsigned long)RSTTS_LANGVOICEVER_MINOR(langv),
                  voicev,
                  (unsigned long)RSTTS_LANGVOICEVER_MAJOR(voicev), (unsigned long)RSTTS_LANGVOICEVER_MINOR(voicev));
    } else {
        wxLogWarning("[Speech] rsttsGetLanguageVoiceVersion failed (code=%d)", verRes);
    }

    // Test synthesis to confirm voice is operational
    wxLogInfo("[Speech] Testing voice with sample synthesis...");
    int testResult = rsttsSynthesize(m_rstts, "test", "text");
    if (RSTTS_ERROR(testResult)) {
        const char* tmsg = rsttsGetErrorMessage(m_rstts);
        wxLogWarning("[Speech] Test synthesis failed (code=%d, msg='%s') - voice may not be properly loaded", testResult, tmsg ? tmsg : "(none)");
        // Continue anyway - some engines may fail test but work in practice
    } else {
        wxLogInfo("[Speech] Test synthesis succeeded");
    }

    // Audio and callback already set earlier in init sequence
    m_thread = std::thread(&Speech::ThreadProc, this);
    wxLogInfo("[Speech] Thread created");
    return true;
}

void Speech::Term()
{
    if (m_thread.joinable()) {
        m_quit = true;
        m_queue.Enqueue("");
        m_thread.join();
    }
    m_audio.Close();
    if (m_rstts != nullptr) {
        rsttsFree(m_rstts);
        m_rstts = nullptr;
    }
}

float Speech::GetSpeed()
{
    float speed = -1.0f;
    int result = rsttsGetSpeed(m_rstts, &speed);
    return RSTTS_SUCCESS(result) ? speed : -1.0f;
}

bool Speech::SetSpeed(float value)
{
    int result = rsttsSetSpeed(m_rstts, static_cast<float>(RSTTS_SPEED_DEFAULT) + value);
    return RSTTS_SUCCESS(result);
}

float Speech::GetVolume()
{
    float volume = -1.0f;
    int result = rsttsGetVolume(m_rstts, &volume);
    return RSTTS_SUCCESS(result) ? volume : -1.0f;
}

bool Speech::SetVolume(float value)
{
    int result = rsttsSetVolume(m_rstts, value);
    return RSTTS_SUCCESS(result);
}

void Speech::Speak(std::string text)
{
    wxLogInfo("[Speech] Queuing text for TTS: '%s' (len=%zu)", text.c_str(), text.length());
    m_queue.Enqueue(text);
}

void Speech::Stop()
{
    if (m_rstts != nullptr) {
        rsttsStop(m_rstts);
    }
    m_audio.Stop();
}

void Speech::ThreadProc()
{
    wxLogInfo("[Speech] ThreadProc started, waiting for text...");
    while (!m_quit) {
        wxLogInfo("[Speech] Waiting for dequeue...");
        std::string text = m_queue.Dequeue();
        wxLogInfo("[Speech] Dequeued text: '%s' (len=%zu, empty=%d, quit=%d)", 
                  text.c_str(), text.length(), text.empty(), m_quit);
        if (!text.empty() && !m_quit) {
            wxLogInfo("[Speech] Synthesizing: '%s' (len=%zu)", text.c_str(), text.length());
            rsttsSynthesize(m_rstts, text.c_str(), "text");
            wxLogInfo("[Speech] Synthesis complete");
        } else {
            wxLogInfo("[Speech] Skipping synthesis (empty=%d, quit=%d)", text.empty(), m_quit);
        }
    }
    wxLogInfo("[Speech] ThreadProc exiting");
}

void Speech::TTSAudioCallback(RSTTSInst inst, const void* audiodata, size_t audiodatalen, void* userptr)
{
    (void)inst;
    Speech* pThis = (Speech*)userptr;
    static int callCount = 0;
    if (callCount < 5) {
        wxLogInfo("[Speech] AudioCallback invoked: %zu bytes (call #%d)", audiodatalen, ++callCount);
    }
    pThis->m_audio.Write(audiodata, static_cast<unsigned long>(audiodatalen / kSampleSize));
}
#endif
