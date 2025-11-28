# Contract: Pronunciation Modifier (Clevy Key)

## Purpose
Holding the Clevy/speaker key changes pronunciation for certain letters (e.g., C → K vs. S) per layout rules.

## Inputs
- Modifier: `Key::WinCmd` held
- Key press: letter key (e.g., C)

## Behavior
- Layout entries include alternate paths when modifier is active.
- On key up, buffers update accordingly; audio/speech reflect alternate pronunciation.

## Tests
- With modifier held, pressing C maps to alternate sound; without modifier, default mapping.
