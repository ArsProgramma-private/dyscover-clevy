# ABI Migration Checklist & Release Notes Template

This file provides a checklist and a template to use when upgrading the project's C++ standard or otherwise making changes which can affect ABI compatibility for downstream consumers.

## Goals
- Make ABI-impacting changes visible and traceable in release notes.
- Provide a repeatable checklist for validating ABI compatibility.
- Help downstream integrators understand required changes and mitigations.

## When to use
Use this checklist when any change modifies public API/ABI, including (but not limited to):
- Changing struct layouts used in public headers
- Changing calling conventions or symbol visibility
- Adding/removing virtual functions in base public classes
- Modifying third-party libraries used by the build to versions with different ABIs

## Pre-release checklist (developer)
- [ ] Identify which public headers and exported symbols changed
- [ ] Run ABI check tools (e.g. abi-compliance-checker, gcc -fdump-lang-something, or platform-specific tools) against previous release
- [ ] Run existing unit & integration tests, check for regressions in linking/behavior with previous ABI
- [ ] If packaging binaries, validate runtime loading and basic functionality on the lowest supported distro/toolchain
- [ ] Prepare migration notes and point out any direct API/ABI changes to consumers

## Release notes template
Title: "BREAKING CHANGES: C++17 upgrade — ABI notes"

Body:
- Summary: This release moves the project to require C++17 as a minimum. This change may affect downstream consumers linking against older binary artifacts in certain configurations.
- What changed: Listing of major ABI-relevant changes (e.g. library symbol version changes, removed legacy APIs, changed header layouts)
- Impact & Risk: Short note about what may break (linker errors, missing symbols, changed initializer layouts) and which consumers are most likely to be affected.
- Recommended actions for integrators:
  - Rebuild downstream projects using the new minimum toolchains (GCC >= 9, Clang >= 10, MSVC >= 16.8)
  - Run integration tests and smoke tests to confirm runtime behavior
  - Report any missing compatibility issues as new issues with steps to reproduce

## Post-release follow-ups
- [ ] Track reported regressions and triage fixes as needed
- [ ] Consider providing compatibility shims if many downstream users are affected
- [ ] Update packaging metadata and distribution repositories to label the new minimum requirements
