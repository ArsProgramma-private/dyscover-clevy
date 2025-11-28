# Debug TTS Logging Patch

This patch adds debug logging to help diagnose why TTS (words/sentences) isn't working on Windows.

## Apply This Patch

Add these logging statements to `src/Core.cpp`:

### 1. After line 43 (in Core constructor, after Init() try-catch):

```cpp
    } catch (...) {
        wxLogWarning(LOG_TAG_SPEECH "Failed to initialize speech engine");
    }
    // ADD THIS:
    wxLogInfo(LOG_TAG_SPEECH "Speech engine initialized: m_pSpeech=%p", m_pSpeech);
```

### 2. After line 203 (in word speaking for Space):

```cpp
            if (!m_wordSpeechBuffer.empty() && m_pConfig->GetWords())
            {
                // ADD THIS:
                wxLogInfo(LOG_TAG_CORE "Speaking word (Space): '%s' (Words=%d)", 
                          m_wordSpeechBuffer.c_str(), m_pConfig->GetWords());
                m_pSpeech->SetSpeed(static_cast<float>(m_pConfig->GetSpeed()));
                m_pSpeech->Speak(m_wordSpeechBuffer);
            }
```

### 3. After line 227 (in sentence speaking):

```cpp
            if (!m_sentenceSpeechBuffer.empty() && m_pConfig->GetSentences())
            {
                // ADD THIS:
                wxLogInfo(LOG_TAG_CORE "Speaking sentence: '%s' (Sentences=%d)", 
                          m_sentenceSpeechBuffer.c_str(), m_pConfig->GetSentences());
                m_pSpeech->Speak(m_sentenceSpeechBuffer);
            }
```

### 4. After line 268 (in character accumulation):

```cpp
                m_wordSpeechBuffer.append(chars);
                m_sentenceSpeechBuffer.append(chars);
                // ADD THIS:
                wxLogInfo(LOG_TAG_CORE "Accumulated char: '%s', wordBuf='%s', sentenceBuf='%s'",
                          chars.c_str(), m_wordSpeechBuffer.c_str(), m_sentenceSpeechBuffer.c_str());
            }
```

## After Rebuilding

1. Rebuild the Windows debug build
2. Run Dyscover.exe from command line to see console output:
   ```cmd
   cd build-windows-Debug\Debug
   Dyscover.exe
   ```
3. Check the console for these log messages:
   - Speech engine init status
   - Character accumulation (should show letters being added to buffers)
   - Word/sentence speaking (should trigger on Space or punctuation)

## Expected Console Output

**When typing "hello world" + Space:**
```
[Speech] Speech engine initialized: m_pSpeech=0x...
[Core] Accumulated char: 'h', wordBuf='h', sentenceBuf='h'
[Core] Accumulated char: 'e', wordBuf='he', sentenceBuf='he'
[Core] Accumulated char: 'l', wordBuf='hel', sentenceBuf='hel'
[Core] Accumulated char: 'l', wordBuf='hell', sentenceBuf='hell'
[Core] Accumulated char: 'o', wordBuf='hello', sentenceBuf='hello'
[Core] Speaking word (Space): 'hello' (Words=1)
[Core] Accumulated char: 'w', wordBuf='w', sentenceBuf='hello w'
[Core] Accumulated char: 'o', wordBuf='wo', sentenceBuf='hello wo'
[Core] Accumulated char: 'r', wordBuf='wor', sentenceBuf='hello wor'
[Core] Accumulated char: 'l', wordBuf='worl', sentenceBuf='hello worl'
[Core] Accumulated char: 'd', wordBuf='world', sentenceBuf='hello world'
[Core] Speaking word (Space): 'world' (Words=1)
```

## Common Issues to Look For

1. **No "Accumulated char" messages** → Key translation is failing, returning empty strings
2. **"Speaking word" shows Words=0** → Config not properly reading Words setting
3. **Buffer shows empty '' in "Speaking word"** → Buffers not accumulating properly
4. **Speech engine pointer is null** → TTS engine failed to initialize

## Troubleshooting Based on Output

### If no character accumulation:
- Check that `m_pPlatformKeyboardHandler->translate()` returns non-empty strings
- Check that `m_pKeyboard->TranslateKeyStroke()` fallback works

### If accumulation works but no speaking:
- Check that `m_pConfig->GetWords()` returns `true` (should log `Words=1`)
- Check that `m_pSpeech->Speak()` is actually called
- Add logging in `Speech::Speak()` to verify queue enqueue

### If Speech::Speak() is called but no audio:
- Check TTS engine initialization in `Speech::Init()`
- Check that worker thread started successfully
- Add logging in `Speech::ThreadProc()` to see if text is dequeued
- Check that `rsttsSynthesize()` is called with the text
