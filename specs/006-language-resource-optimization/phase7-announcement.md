# Phase 7 Switchover Announcement: Layout-Based Resource Structure

**Date**: 2025-11-27  
**Feature**: 006-language-resource-optimization (Phase 2 enhancement)  
**Status**: DEFAULT ENABLED (`USE_LAYOUT_STRUCTURE=ON`)

## Summary
The hierarchical layout resource structure under `res/layouts/` is now the default build path. The legacy flat manifest mode remains available with `-DUSE_LAYOUT_STRUCTURE=OFF` for regression comparison and rollback until cleanup (Phase 3).

## What Changed
- Default flag flipped: `USE_LAYOUT_STRUCTURE` ON by default in `CMakeLists.txt`.
- All layout modules compiled and registered via `LayoutRegistry`.
- Migration script executed successfully (5 layouts, 263 audio files, 30 TTS files copied; 0 errors).
- Documentation updated: `developer-guide.md`, `README.md`, `quickstart.md`.

## Developer Actions
| Action | Required? | Command / Path |
|--------|-----------|----------------|
| Pull latest changes | Yes | `git pull origin 006-language-resource-optimization` |
| Build default structure | Yes | `cmake -B build . && cmake --build build` |
| Legacy comparison build | Optional | `cmake -B build-legacy -DUSE_LAYOUT_STRUCTURE=OFF .` |
| Report issues | Yes | Open issue with label `layout-migration` |

## Rollback Procedure
If a critical issue is discovered:
```bash
cmake -B build-rollback -DUSE_LAYOUT_STRUCTURE=OFF .
# Notify maintainers and attach logs
```

## Monitoring Plan (Week 1)
- CI dual-build workflow (legacy + layout) validates parity while legacy path still supported.
- Track build time & config time deltas (target: <10% overhead).  
- Watch for runtime layout selection errors / missing audio.
- Collect developer feedback mid-week (T071) and end of week.

## Upcoming (Phase 3 Cleanup Criteria)
Cleanup will proceed when:
- No critical issues for 7 consecutive days.
- CI green for both structures for the monitoring window.
- Developer feedback indicates confidence (≥80% positive).

## Accessibility Note
New layout modules were created with accessibility in mind; manual validation with assistive technologies is still recommended.

## Contacts
- Maintainer: <owner>@example.com
- Escalation: #dyscover-core Slack channel

---
Please file any observations promptly to accelerate safe cleanup.
