# Tracking: Layout Structure Switchover Monitoring

**Feature**: 006-language-resource-optimization (Phase 2)  
**Window**: 2025-11-27 → 2025-12-04  
**Objective**: Validate stability & performance of default `USE_LAYOUT_STRUCTURE=ON` mode before legacy cleanup.

## Metrics
| Metric | Target | Baseline (Legacy) | Current (Layout) | Status |
|--------|--------|-------------------|------------------|--------|
| CMake Configure Time | < +5s delta | 2.3s | 2.6s | ✅ |
| Clean Build Time | < +10% delta | 45s | 48s | ✅ |
| Incremental Build | ~unchanged | 5.0s | 5.5s | ✅ |
| Startup Time | No increase | 1.2s | 1.2s | ✅ |
| Layout Count Discovered | = expected (5) | 5 | 5 | ✅ |
| Audio Files Registered | = migration report (263) | 263 | 263 | ✅ |
| TTS Files Present | = migration report (30) | 30 | 30 | ✅ |

## CI Status Log
| Date | Legacy Build | Layout Build | Tests | Notes |
|------|--------------|--------------|-------|-------|
| 2025-11-27 | ✅ | ✅ | ✅ | Switchover commit, warnings only |
| 2025-11-28 | ☐ | ☐ | ☐ | (to be filled) |
| 2025-11-29 | ☐ | ☐ | ☐ | |
| 2025-11-30 | ☐ | ☐ | ☐ | |
| 2025-12-01 | ☐ | ☐ | ☐ | |
| 2025-12-02 | ☐ | ☐ | ☐ | |
| 2025-12-03 | ☐ | ☐ | ☐ | |
| 2025-12-04 | ☐ | ☐ | ☐ | Decision checkpoint |

Legend: ✅ pass, ❌ fail, ⚠ flaky, ☐ pending entry.

## Issue Triage
| ID | Severity | Description | Observed In | Action | Status |
|----|----------|-------------|-------------|--------|--------|
| - | - | (none yet) | - | - | - |

## Developer Feedback
| Date | Positive (%) | Themes | Actions |
|------|--------------|--------|---------|
| Mid-week | (pending) | | |
| End-week | (pending) | | |

## Rollback Triggers
- ≥2 consecutive CI failures (layout mode) impacting release readiness.
- Data corruption in layout registry (missing or misrouted entries).
- Performance regression >15% sustained across two days.

## Next Actions
| Task | Owner | Due |
|------|-------|-----|
| Populate CI entries daily | Automation/Script | Daily 09:00 UTC |
| Mid-week feedback survey | Maintainer | 2025-12-01 |
| End-week decision review | Maintainer | 2025-12-04 |

## Accessibility Considerations
Monitor for any regressions in keyboard navigation or resource labeling introduced by modular layout sources. Recommend targeted audit before cleanup.

---
Update this file daily; commit changes with message prefix `tracking: layout-switchover`.
