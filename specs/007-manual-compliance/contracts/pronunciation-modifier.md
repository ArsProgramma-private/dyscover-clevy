# Contract: Pronunciation Modifier (Alt Key)

## Purpose
Holding the Alt key changes pronunciation for certain letters (e.g., A → aa, Y → j) per layout rules.

## Audit Findings (2025-11-28)
All layouts (Dutch Classic, Dutch Default, Dutch KWeC, Flemish Classic, Flemish Default) include Alt-modified mappings:
- **Vowels**: Alt+A → "aa.wav", Alt+E → "ee.wav", Alt+I → "ie.wav", Alt+O → "oo.wav", Alt+U → "uu.wav"
- **Y**: Alt+Y → "j.wav" (in Dutch layouts)
- Layout entries have `alt=true` flag; `TranslateKey` already respects this modifier.

## Inputs
- Modifier: Alt key held (not WinCmd; WinCmd triggers selection read-out)
- Key press: letter key (e.g., A, E, I, O, U, Y)

## Behavior
- Layout entries with `alt=true` provide alternate keystroke output and/or sound.
- On key up, buffers update with alternate pronunciation; audio/speech reflect alternate mapping.

## Tests
- With Alt held, pressing A maps to "aa.wav"; without Alt, default "a.wav".
- Unit test: `TranslateKey(Key::A, false, false, false, true, ...)` returns "aa.wav" vs. default "a.wav".
