# Feature Specification: Upgrade C++ Standard to C++17

**Feature Branch**: `001-upgrade-cpp17`  
**Created**: November 23, 2025
**Status**: Draft
**Input**: User description: "We need to update this solution to a current C++ standard, at least to C++ 17"

## Summary *(mandatory)*

Update the project to require and use C++17 as the minimum language standard across all supported platforms (Windows, Linux, macOS). The goal is to modernize the codebase, enable safer/clearer idioms, and make future maintenance easier.

This specification is intentionally implementation-agnostic: it describes what must change, why, and how success will be verified — not the detailed implementation steps.

## User Scenarios & Testing *(mandatory)*

### User Story 1 — Developer build (Priority: P1)

As a developer, I want the project to build with C++17 so that I can use modern language features and the project remains maintainable with current toolchains.

Independent Test: Rebuild the project on each supported platform using the updated CMake configuration and confirm a successful build and run of the test-suite.

Acceptance Scenarios:
1. **Given** the repo is updated to C++17, **When** a developer runs the standard build (CMake + build), **Then** the project builds successfully with no regressions in unit/integration tests.
2. **Given** the updated standard, **When** the project is compiled with older compilers that do not meet the minimum version, **Then** configuration should fail with a clear diagnostic explaining the required compiler minimum. Minimum supported compiler versions (CI targets): GCC >= 9 (Linux), Clang >= 10 (macOS), MSVC (Visual Studio 2019) >= 16.8 (Windows).

---

### User Story 2 — CI / packaging (Priority: P2)

As a release engineer, I want CI and packaging procedures to reflect the new standard so that the project's releases are reproducible and supported compilers are validated automatically.

Independent Test: Update CI matrix to include supported compiler versions and confirm all builds pass on the CI provider.

Acceptance Scenarios:
1. **Given** CI configuration is updated, **When** a change is pushed, **Then** CI validates builds across declared compilers and reports results.
2. **Given** packaging steps, **When** building a release artifact, **Then** the artifact is buildable with the declared supported toolchains and passes smoke tests.

---

### User Story 3 — Backwards compatibility policy (Priority: P3)

As a product manager, I need a clear backwards-compatibility policy for binary distribution and third-party integrations so that the impact of changing the language standard is known.

Independent Test: Confirm there are no ABI-breaking changes for typical distribution formats or produce a documented migration plan if ABI changes are unavoidable.

Acceptance Scenarios:
1. **Given** the project moves to C++17, **When** packaging binaries for distribution, **Then** package metadata documents the minimum supported runtime environment and any compatibility notes.
2. **Given** a downstream integrator depends on the library, **When** they attempt to build/link, **Then** documentation clearly lists required compiler/ABI constraints.

---

## Functional Requirements *(mandatory)*

- **FR-CPP-001**: The build system MUST set C++17 (or higher) as the project minimum. CMake must express this via `CMAKE_CXX_STANDARD` and appropriate checks.
- **FR-CPP-002**: The project MUST fail configuration with a clear message when the detected compiler does not support the required C++ standard or required language features.
- **FR-CPP-003**: The CI matrix MUST include at least one supported compiler for each supported OS (e.g., Ubuntu/GCC, macOS/clang, Windows/MSVC) that supports C++17.
- **FR-CPP-004**: All unit and integration tests MUST pass unchanged (functionality maintained); any failing tests must be triaged and fixed before merging.
- **FR-CPP-005**: The repository documentation (README / build instructions) MUST be updated to reflect the new minimum C++ standard and the minimal supported toolchains.
- **FR-CPP-006**: The build artifacts and packaging process MUST be verified on supported platforms after the change.

## Non-Functional Requirements

- **NFR-CPP-001**: The change MUST not increase the project's runtime memory usage by more than 5% for steady-state behaviour.
- **NFR-CPP-002**: Build time MAY increase temporarily during developer transition; CI pass rate must remain >= 95%.

## Key Entities / Data Model

Not applicable — this is a build/tooling change. Any code-level data changes MUST be documented in the scope where they occur (e.g., source headers changed to use modern facilities).

## Success Criteria *(mandatory, measurable)*

- **SC-CPP-001**: A clean build succeeds on all supported platforms using the declared toolchain(s) and C++17 as the minimum.
- **SC-CPP-002**: Existing unit and integration tests pass in CI for all supported platforms (no more than 2% new failures caused by the change; ideally 0%).
- **SC-CPP-003**: Developer documentation updated with build instructions showing C++17 minimum and sample compiler versions on Linux/macOS/Windows.
- **SC-CPP-004**: CI pipeline updated and green for the branch and for a test release artifact build.

## Assumptions

- The majority of the source base is compatible with C++17 with only minor code changes necessary.
- CI has sufficient flexiblity to spin up modern toolchain images; if not, the project owner will provide updated runners or images.
- No third-party library in the vendor tree requires a lower C++ standard to be built; if they do, the compatibility problem will be addressed as part of the change.

## Risks & Mitigations

- Risk: Some platforms or compilers used by maintainers may not support C++17.  
  Mitigation: Validate minimum supported compiler versions and add helpful pre-checks in CMake configuration.

- Risk: ABI or runtime compatibility with downstream packaging consumers may be impacted.  
  Mitigation: Document compatibility clearly. If ABI changes are required, flag in release notes and offer migration guidance.

## Open Questions / Clarifications (max 3)

1. [NEEDS CLARIFICATION: Minimum supported compiler versions and CI targets] — Which minimum compiler versions should the project guarantee on each platform (Linux/GCC, macOS/clang, Windows/MSVC)?

2. Policy: Require at least C++17 but accept newer standards (C++20+) where available; the build will assert minimum support and allow newer standards when present, with use of newer features guarded or reviewed.

3. Policy: ABI changes are allowed but must be documented. The project will permit ABI changes as part of modernization but require clear release notes and migration guidance for downstream integrators.

## Next steps

1. Resolve the three clarifications above (Q1–Q3).
2. Update `CMakeLists.txt` to set `CMAKE_CXX_STANDARD 17` and add compiler checks and helpful diagnostic messages.
3. Update CI to include at least one modern compiler per OS that supports C++17.
4. Run full test-suite, mark failing tests, and fix regressions.
5. Update docs and packaging, then request a review.
