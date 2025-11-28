# Troubleshooting: TTS Not Working for Words/Sentences on Windows

## Problem Description

On Windows debug builds, only single letter sounds play via audio files, but the TTS (text-to-speech) engine does not speak words or sentences.

## Root Cause Analysis

Based on code review, the most likely causes are:

### 1. Configuration Settings

The TTS for words and sentences is controlled by configuration checkboxes in the preferences dialog:
- `Config::GetWords()` - Controls word TTS
- `Config::GetSentences()` - Controls sentence TTS

**Check:** Open preferences dialog and verify both checkboxes are enabled.

**Location:** `src/Config.cpp` lines 111-131
- Default values: `kWordsDefaultValue = true` and `kSentencesDefaultValue = true`
- Stored in: `ClevyDyscover.ini` under keys `/Dyscover/Words` and `/Dyscover/Sentences`

**Windows Config File Location:**
- Typically in user's home directory or AppData
- File name: `ClevyDyscover.ini`

### 2. Speech Engine Initialization

The TTS engine (`librstts`) must initialize successfully:

**Check in Core.cpp constructor (line 36-43):**
```cpp
try {
    m_pSpeech->Init(GetTTSDataPath(), TTS_LANG, TTS_VOICE);
    m_pSpeech->SetVolume(RSTTS_VOLUME_MAX);
} catch (...) {
    wxLogWarning(LOG_TAG_SPEECH "Failed to initialize speech engine");
}
```

**Possible Issues:**
- TTS data files missing from `tts/data/` directory
- Wrong TTS_LANG or TTS_VOICE defined
- `librstts-2.dll` not found or incompatible

**Verification:**
1. Check that `tts/data/` directory exists in build output
2. Check that `librstts-2.dll` is in the same directory as `Dyscover.exe`
3. Look for warning message in debug output: `[Speech] Failed to initialize speech engine`

### 3. Text Buffering Logic

Words and sentences are buffered differently than letters:

**Letter sounds** (Core.cpp line 193-197):
- Played immediately via `SoundPlayer::PlaySoundFile()`
- Uses audio files from `audio/` directory
- Controlled by `Config::GetLetters()`

**Word TTS** (Core.cpp line 206-209, 218-220):
- Accumulated in `m_wordSpeechBuffer`
- Spoken when Space, Tab, Enter, or sentence-ending punctuation is pressed
- Uses `Speech::Speak()` which queues text to librstts
- Controlled by `Config::GetWords()`

**Sentence TTS** (Core.cpp line 223-226):
- Accumulated in `m_sentenceSpeechBuffer`
- Spoken when sentence-ending punctuation (`.`, `!`, `?`) is pressed
- Uses `Speech::Speak()` which queues text to librstts
- Controlled by `Config::GetSentences()`

**Buffering depends on** (Core.cpp lines 261-273):
```cpp
for (KeyStroke ks : translation.keystrokes)
{
    std::string chars;
    if (m_pPlatformKeyboardHandler)
    {
        KeyModifiers mods; mods.shift = ks.shift; mods.ctrl = ks.ctrl; mods.alt = ks.alt;
        chars = m_pPlatformKeyboardHandler->translate(ks.key, mods);
    }
    if (chars.empty())
    {
        chars = m_pKeyboard->TranslateKeyStroke(ks.key, ks.shift, ks.ctrl);
    }
    m_wordSpeechBuffer.append(chars);
    m_sentenceSpeechBuffer.append(chars);
}
```

**Issue:** If key translation returns empty strings, buffers won't accumulate text.

### 4. Speech Queue and Thread

The Speech class uses a queue and worker thread (Speech.cpp line 120-125):
```cpp
void Speech::ThreadProc()
{
    while (!m_quit) {
        std::string text = m_queue.Dequeue();
        rsttsSynthesize(m_rstts, text.c_str(), "text");
    }
}
```

**Possible Issues:**
- Thread not started (check `Init()` succeeded)
- Queue blocking/deadlock
- `rsttsSynthesize()` failing silently

## Diagnostic Steps

### Step 1: Check Configuration File

**Windows PowerShell:**
```powershell
Get-Content "$env:USERPROFILE\ClevyDyscover.ini" | Select-String -Pattern "Words|Sentences"
```

**Expected Output:**
```
/Dyscover/Words=true
/Dyscover/Sentences=true
```

**If not found or set to false:**
1. Open Dyscover
2. Go to Preferences
3. Check "Speak words" and "Speak sentences"
4. Click OK
5. Restart Dyscover

### Step 2: Check TTS Files

**PowerShell:**
```powershell
# From build directory (e.g., build-windows-Debug)
Get-ChildItem -Path ".\Debug\tts\data" -Recurse | Select-Object Name
Get-ChildItem -Path ".\Debug" -Filter "librstts*.dll"
```

**Expected:**
- Multiple TTS data files in `tts/data/`
- `librstts-2.dll` in executable directory

**If missing:**
- Rebuild with proper CMake configuration
- Ensure `BUILD_WITH_LIBRSTTS=ON` in CMakeCache.txt
- Check build output for TTS file copying messages

### Step 3: Enable Debug Logging

Add debug output to Core.cpp to trace TTS calls:

**In Core::OnKeyEvent(), after line 206:**
```cpp
if (!m_wordSpeechBuffer.empty() && m_pConfig->GetWords())
{
    wxLogInfo("[DEBUG] Speaking word: '%s'", m_wordSpeechBuffer.c_str());
    m_pSpeech->SetSpeed(static_cast<float>(m_pConfig->GetSpeed()));
    m_pSpeech->Speak(m_wordSpeechBuffer);
}
```

**After line 223:**
```cpp
if (!m_sentenceSpeechBuffer.empty() && m_pConfig->GetSentences())
{
    wxLogInfo("[DEBUG] Speaking sentence: '%s'", m_sentenceSpeechBuffer.c_str());
    m_pSpeech->Speak(m_sentenceSpeechBuffer);
}
```

Rebuild and check debug console output when typing words and sentences.

### Step 4: Check Speech Init

Add logging to Speech::Init():

**In Speech.cpp after line 68:**
```cpp
m_thread = std::thread(&Speech::ThreadProc, this);
wxLogInfo("[Speech] TTS engine initialized successfully (lang=%s, voice=%s)", lang, voice);
return true;
```

**In Speech.cpp ThreadProc after line 124:**
```cpp
wxLogInfo("[Speech] Synthesizing text: '%s' (length=%zu)", text.c_str(), text.length());
rsttsSynthesize(m_rstts, text.c_str(), "text");
```

## Quick Fix Checklist

- [ ] Verify preferences: Words and Sentences checkboxes are checked
- [ ] Check `ClevyDyscover.ini` has `Words=true` and `Sentences=true`
- [ ] Verify `librstts-2.dll` exists in executable directory
- [ ] Verify `tts/data/` directory has content
- [ ] Check CMakeCache.txt shows `BUILD_WITH_LIBRSTTS:BOOL=ON`
- [ ] Review debug logs for "[Speech] Failed to initialize" warnings
- [ ] Test with debug logging enabled to trace buffer accumulation
- [ ] Verify keys are being translated to characters (not empty strings)

## Common Solutions

### Solution 1: Reset Configuration
Delete `ClevyDyscover.ini` and restart Dyscover. This will recreate with default values (all features enabled).

### Solution 2: Rebuild with Fresh Configuration
```powershell
Remove-Item -Recurse -Force build-windows-Debug
.\build-windows.ps1 -Config Debug -Package Off
```

### Solution 3: Manual DLL Copy
If automated build didn't copy librstts:
```powershell
Copy-Item lib\rstts\platforms\x86_64-pc-win64\librstts-2.dll build-windows-Debug\Debug\
```

### Solution 4: Check Keyboard Layout Registration
If keys aren't being translated to text, check that layouts are registered:
- In App.cpp, layouts should be registered during startup
- TranslateKey() should return non-empty keystrokes with character mappings

## Related Code Sections

- **Config:** `src/Config.cpp` (GetWords/GetSentences)
- **TTS Init:** `src/Core.cpp` lines 36-43
- **Word buffering:** `src/Core.cpp` lines 206-220
- **Sentence buffering:** `src/Core.cpp` lines 221-232
- **Character accumulation:** `src/Core.cpp` lines 261-273
- **Speech engine:** `src/Speech.cpp` (Init, Speak, ThreadProc)
- **CMake TTS config:** `CMakeLists.txt` lines 308-355

## See Also

- `specs/007-manual-compliance/` - Manual compliance feature docs
- `tests/unit/PronunciationModifierTest.cpp` - Test showing TTS integration
- `docs/AUDIO_LICENSING.md` - Audio file licensing info
