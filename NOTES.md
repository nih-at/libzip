# Google Patch Rewards — libzip local draft notes

**Date:** 2026-09-11 (America/Chicago)  
**Do not claim yet:** need upstream merge + ≥30 days, then https://bughunters.google.com/report/patch_rewards

## Chosen target + why

- **Project:** libzip (Tier-1 adjacent / widespread archive parser — Chromium, KDE, ImageMagick; Google Patch Rewards memory-safety track)
- **Upstream:** https://github.com/nih-at/libzip (prefer `main`)
- **Local clone:** `/workspace/google-patch-libzip`
- **Branch:** `local/zip-buffer-fbounds-safety` (from `main`)
- **Why this target:**
  1. Widely used ZIP archive library on untrusted-input parse paths (central directory / local headers / extra fields via `_zip_buffer_*`).
  2. Clear, mergeable first-CL scope: **one** internal buffer+size pair — `struct zip_buffer` (`data` ↔ `size`) — textbook `__sized_by`, not a whole-library sweep.
  3. Same pattern as libpng / libwebp / giflib / lz4 / zstd: **inert macros** when the flag is off; experimental Clang `-fbounds-safety` only when explicitly enabled.
  4. Stable layout preserved (no field reorder); annotations link `data` to its **capacity** (`size`).
  5. Not already done upstream (no `__sized_by` / `-fbounds-safety` in tree; no overlapping open annotation PRs).
  6. AI CONTRIBUTING gate clear (no CONTRIBUTING.md; no AI ban in README / SECURITY / THANKS / `.github`).

**Why `struct zip_buffer` over public `zip_buffer_fragment` or `zip_extra_field`:** Internal `zip_buffer` is the shared bounds-checked byte cursor used across dirent/EOCD/extra-field parsing of untrusted ZIP bytes. Public fragment structs and `zip_extra_field` are natural follow-ups; this CL stays at the single textbook sized_by gap.

**ABI / layout note:** Field order is preserved (pointer before size). Only the `_zip_buffer_new` assign order is capacity-then-pointer.

## Security benefit

`struct zip_buffer` backs `_zip_buffer_get` / `_zip_buffer_peek` and related helpers that walk untrusted ZIP metadata. Callers already track capacity in `size`, but the compiler cannot see that `data` is bounded by that field.

This draft:

1. Introduces `lib/zip_bounds_safety.h` with `ZIP_SIZED_BY` / `ZIP_SIZED_BY_OR_NULL` / `ZIP_COUNTED_BY*` (empty by default).
2. Annotates **only** `struct zip_buffer.data` → `ZIP_SIZED_BY(size)` (capacity-first bound).
3. Keeps existing field order. Makes `_zip_buffer_new` assign **capacity before pointer**.
4. Wires optional CMake `ENABLE_FBOUNDS_SAFETY` (default **OFF**) → `-DZIP_SUPPORT_FBOUNDS_SAFETY` + `-fbounds-safety` on the `zip` / `zip_nonrandom` targets.

**Default builds are unchanged:** macros expand to nothing; no new runtime checks without the experimental flag. Header is internal-only (not installed); only `zip.h` remains the public install surface.

## Files changed

| File | Change |
|------|--------|
| `lib/zip_bounds_safety.h` | **New** — inert / Clang bounds macros |
| `lib/zipint.h` | Include header; annotate `struct zip_buffer.data` |
| `lib/zip_buffer.c` | Capacity-first assign in `_zip_buffer_new` |
| `CMakeLists.txt` | `ENABLE_FBOUNDS_SAFETY` option OFF |
| `lib/CMakeLists.txt` | Apply `-fbounds-safety` when option ON |
| `NOTES.md` | This file |

## Verified locally 2026-09-11

| Check | Result |
|-------|--------|
| Default `ENABLE_FBOUNDS_SAFETY=OFF` cmake `-DBUILD_DOC=OFF` build (lib + tools) | **PASS** (gcc; `libzip.so` + zipcmp/zipmerge/ziptool) |
| In-tree `ctest` / nihtest regress suite | **Skipped** — `nihtest` not installed on this box (`CMake Warning: nihtest not found, regression testing disabled`) |
| `ENABLE_FBOUNDS_SAFETY=ON` | **Not feasible on this box** — needs Clang with `-fbounds-safety` / `ptrcheck.h` |

## How to build / test

Default (macros inert — must stay green):

```sh
cmake -S . -B build -DENABLE_FBOUNDS_SAFETY=OFF \
  -DBUILD_DOC=OFF -DBUILD_EXAMPLES=OFF -DBUILD_OSSFUZZ=OFF
cmake --build build -j
ctest --test-dir build --output-on-failure
```

With experimental bounds-safety toolchain (maintainers / CI; **not** available on this box — no Clang/`ptrcheck.h`):

```sh
cmake -S . -B build-fbs -DENABLE_FBOUNDS_SAFETY=ON \
  -DCMAKE_C_COMPILER=<clang-with-fbounds-safety> \
  -DBUILD_DOC=OFF
cmake --build build-fbs -j
```

## Upstream submit plan

1. Open a focused GitHub PR against `nih-at/libzip` branch **`main`**.
2. Proposed title: `buffer: add optional -fbounds-safety annotations for struct zip_buffer`
3. Frame as secure-by-design / Safe Buffers-style systematization of the existing data+size pair; cite libwebp/libpng/lz4/zstd prior art and Google Patch Rewards memory-safety goals.
4. Emphasize: default build behavior unchanged; flag OFF; no PoC / no CVE claim; internal layout field order unchanged.
5. Do **not** claim on https://bughunters.google.com/report/patch_rewards until **merge + ≥30 days**.

## Follow-ups (separate CLs)

- `struct zip_extra_field` `data`/`size` (already capacity-before-pointer at `_zip_ef_new`)
- Public `struct zip_buffer_fragment` `data`/`length`
- Other internal buffer+size pairs on the parse path as diagnostics under a real `-fbounds-safety` build dictate

## AI gate

- **No `CONTRIBUTING.md`** in upstream tree.
- Searched README, SECURITY.md, THANKS, TODO.md, `.github/` — **no AI / LLM / Copilot ban**.
- **AI gate: CLEAR** (no ban found).

## Status

**LOCAL DRAFT ONLY** — commit on `local/zip-buffer-fbounds-safety`. Do **not** push/PR from this agent run. Still **no claim** until merge + ≥30 days unreverted.
