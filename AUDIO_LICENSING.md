# Audio and TTS Licensing Information

## Overview

This document describes the licensing and source of audio files used in the Dyscover Clevy application.

**Last Updated:** November 25, 2025

---

## TTS (Text-to-Speech) System

### TTS Engine: rSpeak SDK (ReadSpeaker)

**Status:** ⚠️ **COMMERCIAL/PROPRIETARY - REQUIRES LICENSE**

- **Copyright:** 
  - © 2017 rSpeak Technologies
  - © 2018-2020 ReadSpeaker B.V.
- **Location:** `lib/rstts/`
- **Platforms:** Windows (x86/x64), Linux (x86/x64), macOS (x86_64)
- **Documentation:** `lib/rstts/release_notes.txt`

### License Details

The application uses an embedded XML license for the rSpeak SDK:

- **License ID:** `e73db530bb7c651ea041eca0eb7faba2`
- **Subject:** "rSpeak SDK SuperLicense"
- **Licensed Product:** SDK rSpeak
- **Licensed Voices:**
  - Ilse (Dutch - nl_nl)
  - Max
  - Gina
  - Veerle (Flemish - nl_be)

**Location in code:** `src/Speech.cpp` (embedded XML license string)

### Voice Data Files

**Status:** ⚠️ **PROPRIETARY - PART OF READSPEAK LICENSE**

- **Location:** `res/data/tts/data/`
- **Active Voices:** Ilse, Veerle
- **File Types:**
  - `.db` - Database files
  - `.fon` - Font/phoneme data
  - `.udb` - User database
  - `.fsa` - Finite state automaton
  - `.fst` - Finite state transducer

These files are proprietary voice data for the rSpeak TTS engine and are covered under the ReadSpeaker license agreement.

---

## Phoneme Audio Files

**Status:** ⚠️ **SOURCE AND LICENSE UNKNOWN**

- **Location:** `res/data/*.wav`
- **File Count:** ~60+ individual phoneme recordings
- **Examples:** `a.wav`, `aa.wav`, `b.wav`, `c.wav`, `ch.wav`, `ee.wav`, etc.
- **Purpose:** Individual phoneme sounds for keyboard feedback

### ⚠️ Action Required

**The source and licensing of these phoneme WAV files is undocumented.** 

Questions to resolve:
1. Were these files custom-recorded for this project?
2. Were they licensed from a third party?
3. Are they derivative works from another source?
4. What are the usage rights and distribution permissions?

**Recommendation:** Document the origin, creator, and license terms for these files.

---

## System Sound Effects

**Status:** ⚠️ **SOURCE AND LICENSE UNKNOWN**

- **Location:** `res/data/*.wav`
- **Files:**
  - `dyscover_bt_connect_positive.wav`
  - `dyscover_bt_connect_positive_with_voice.wav`
  - `dyscover_connect_negative.wav`
  - `dyscover_connect_negative_with_voice.wav`
  - `dyscover_connect_positive.wav`
  - `dyscover_connect_positive_with_voice.wav`

### ⚠️ Action Required

**The source and licensing of these system sounds is undocumented.**

**Recommendation:** Document the origin, creator, and license terms for these files.

---

## Numeric Audio Files

**Status:** ⚠️ **SOURCE AND LICENSE UNKNOWN**

- **Location:** `res/data/*.wav`
- **Files:** `0.wav`, `1.wav`, `2.wav`, `3.wav`, `4.wav`, `5.wav`, `6.wav`, `7.wav`, `8.wav`, `9.wav`
- **Purpose:** Number pronunciation for keyboard feedback

### ⚠️ Action Required

**The source and licensing of these number audio files is undocumented.**

**Recommendation:** Document the origin, creator, and license terms for these files.

---

## Build Configuration

The TTS system can be disabled at build time:

```bash
cmake -DBUILD_WITH_LIBRSTTS=OFF
```

This is useful for CI/CD pipelines or builds that don't require TTS functionality.

---

## Legal Compliance Checklist

- [x] TTS Engine (rSpeak SDK) - Licensed and documented
- [x] Voice Data (Ilse, Veerle) - Covered under rSpeak license
- [ ] **Phoneme WAV files - NEEDS DOCUMENTATION**
- [ ] **System sound effects - NEEDS DOCUMENTATION**
- [ ] **Numeric audio files - NEEDS DOCUMENTATION**

---

## Recommendations for Future Development

1. **Document all audio sources:** Create a manifest with source, creator, date, and license for each audio file
2. **Consider alternatives:** If redistribution rights are unclear, consider:
   - Using open-source TTS engines (espeak, Festival, Pico TTS)
   - Recording/generating new audio with clear licensing
   - Licensing audio from stock audio providers with clear usage rights
3. **License file:** Add a `LICENSE_AUDIO.txt` or similar file with specific terms
4. **Attribution:** If using third-party audio, ensure proper attribution per license requirements

---

## Contact Information

For questions about licensing or to update this document, contact:
- **Project maintainer:** [Contact information needed]
- **ReadSpeaker licensing:** Contact ReadSpeaker B.V. for TTS license terms

---

## References

- ReadSpeaker website: https://www.readspeaker.com/
- rSpeak SDK release notes: `lib/rstts/release_notes.txt`
- License implementation: `src/Speech.cpp`
