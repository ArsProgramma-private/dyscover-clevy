#include "DebugLogger.h"

#include <fstream>
#include <filesystem>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <mutex>
#include <wx/filename.h>
#include <wx/stdpaths.h>

namespace {
    std::mutex g_logMutex;

    // Get a candidate path next to the running executable: <exe-dir>/logs/dyscover_debug.log
    std::string GetLogFilePath() {
        wxFileName exe(wxStandardPaths::Get().GetExecutablePath());
        wxString dir = exe.GetPath();
        wxFileName logPath(wxString::Format("%s/logs", dir).ToStdString());
        // Ensure directory path and final logfile
        std::string fullDir = logPath.GetFullPath().ToStdString();
        std::string file = fullDir + "/dyscover_debug.log";
        return file;
    }

        std::string TimeNowString() {
        using namespace std::chrono;
        auto now = system_clock::now();
        auto t = system_clock::to_time_t(now);
        auto ms = duration_cast<milliseconds>(now.time_since_epoch()) % 1000;

        std::tm buf{};
    #if defined(_MSC_VER)
        // Use secure localtime_s on MSVC
        localtime_s(&buf, &t);
    #elif defined(__unix__) || defined(__APPLE__)
        // Use reentrant localtime_r on POSIX
        localtime_r(&t, &buf);
    #else
        // Fallback: localtime (may be non-thread-safe but should be fine for our small utility)
        std::tm* p = std::localtime(&t);
        if (p) buf = *p;
    #endif

        std::ostringstream ss;
        ss << std::put_time(&buf, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
        }
}

namespace DebugLogger {
    void Append(const std::string& message) {
        std::lock_guard<std::mutex> lock(g_logMutex);
        std::string path = GetLogFilePath();
        // Ensure the directory exists (best-effort). Create parent dir if needed so the
        // file open succeeds even when 'logs' is not present next to the exe.
        try {
            std::filesystem::path p(path);
            if (p.has_parent_path()) {
                std::filesystem::create_directories(p.parent_path());
            }
        } catch (...) {
            // ignore filesystem failures; we'll still try to open the file
        }
        // Try best-effort: open using append mode and write a line with timestamp.
        try {
            std::ofstream ofs(path, std::ios::app | std::ios::out);
            if (!ofs) return; // can't open
            ofs << TimeNowString() << " " << message << "\n";
            ofs.flush();
        } catch (...) {
            // Best-effort - swallow errors
        }
    }
}
