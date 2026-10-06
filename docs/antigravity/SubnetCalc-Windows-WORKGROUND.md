# SubnetCalc-Windows — Workground

Binding attachment of `SubnetCalc-Windows-ANTIGRAVITY-PROMPT.md`. Antigravity runs the master prompt, not this file.
The 13-pillar closeout in the master prompt is the same gate as the checklist below. Do not keep a second, conflicting list.
Related: `SubnetCalc-Windows-IMPLEMENTATION-PLAN.md`, `SubnetCalc-Windows-CI.md`, `SubnetCalc-Windows-TASKLIST.md`.
If this file conflicts with the master prompt, the master prompt wins.

Use this file as the standing rules for every Antigravity IDE session on `SubnetCalc-Windows`.

## Mission

Produce a Windows 7-capable native subnet calculator that matches SubnetCalc-MacOS v2.6.2 calculation behavior, with no .NET and no Visual C++ runtime dependency.

## Evidence rule

- Do not invent Win32 functions, RFC behavior, cloud reserved counts, or test results.
- Cloud reserved counts come from `CloudProfile` in `IPSubnetcalc.swift` of `alsyundawy/SubnetCalc-MacOS`. If a future macOS commit changes them, update `src/engine/cloud.c` and the vectors together.
- A function is done only when a test or a cited manual check shows the result.
- If a Windows 7 API is uncertain, look it up before using it. Preferred references: Microsoft Learn Win32 docs, RFC editor, MinGW-w64 `ucrt-vs-msvcrt` note.

## Stack fence

Allowed:

- C11
- Win32 wide API
- MinGW-w64 `i686-w64-mingw32` and `x86_64-w64-mingw32`
- `msvcrt.dll` as the CRT (already on Windows 7)
- Static libgcc
- System DLLs listed in the implementation plan

Forbidden without a written user override:

- C# / .NET / WinForms / WPF / WinUI
- MSVC `/MD`, UCRT toolchain, VC++ redistributable
- Qt, wx, GTK, Electron, WebView2
- Rust stable current, Go official 1.21+, Python, Java
- `ProcessPrng`, `bcryptprimitives.dll`
- Network calls, telemetry, auto-update

## Session loop

1. Read `IMPLEMENTATION-PLAN.md` and the task being started.
2. Touch only the files named in that task.
3. Write or update the golden vector before changing engine behavior.
4. Build both architectures, or state why one architecture could not be built.
5. Run tests. Paste the command and the exit code in the task note.
6. Fill the pillar checklist below for the files changed. Unchecked means not reviewed, not passed.
7. Stop. Do not start the next phase inside the same edit unless the user asked for it.

## 13-pillar gate

Apply to every engine or UI change. Severity: Critical, High, Medium, Low, Nit.

1. Bug — wrong result, off-by-one prefix, empty range, broken slider sync.
2. Syntax — clean under `-Wall -Wextra -Wpedantic`. No unused result that hides a failure.
3. Runtime — missing control, dialog cancel treated as success, Wine/Win7 path assumptions.
4. Logic — `/31`, `/32`, prefix 0, prefix 128, non-contiguous mask rejected, cloud minimum prefix.
5. Memory — every `malloc` has a matching free on every return path. No unbounded row growth. No `alloca` of user-sized data.
6. Dead code — no unused theme tables, no commented-out blocks committed as logic.
7. Duplicate — one IPv4 parse, one IPv6 parse, one CSV writer. UI must not reimplement math.
8. Circular dependency — `engine/` must not include `ui/` or `windows.h`.
9. Performance — calculation stays on the UI thread only because it is bitwise and bounded. No per-keystroke allocation of the VLSM table larger than the cap.
10. Security — CWE-1236 CSV, CWE-22 path, CWE-190 shift/multiply, CWE-770 cap, CWE-330 CSPRNG. No secrets in the repo.
11. Maintainability — function fits on one screen. Public headers expose structs, not Win32 types.
12. Scalability — IPv6 count uses the big-integer helper. History capped at 100.
13. Readability — names say network/prefix/host. Comments only for RFC or Win7 traps.

A Critical or High finding blocks the task. Nit does not.

## Windows 7 traps already verified

- UCRT is not on a stock Windows 7 image. KB2999226 adds it. Do not depend on that update. Source: MinGW-w64 `ucrt-vs-msvcrt.txt`.
- `bcrypt.dll` and `BCryptGenRandom` exist on Windows 7. `ProcessPrng` does not.
- `long` is 32-bit on both Win32 and Win64. Use `uint32_t` and `uint64_t` from `stdint.h`.
- DPI: call `SetProcessDPIAware`. Do not require the Windows 10 awareness-context API.
- Manifest `supportedOS` GUID for Windows 7: `{35138b9a-5d96-4fbd-8e2d-a2440225f93a}`.

## UI rules

- All visible strings are wide literals or loaded from the resource table.
- Tab order works with keyboard only.
- Slider and edit box write the same model. The model is the source of truth.
- Copy uses `CF_UNICODETEXT` and owned global memory.
- About window names GPL-2.0, Julien Mulot, and Harry Dertin Sutisna Alsyundawy, and links the macOS repo.

## Test rules

- Engine tests are console programs with no Win32 UI.
- Expected values are written first.
- A failing test is a failed task, not a reason to edit the vector.
- Host test build uses the system `gcc` and the same `src/engine` files.

## Commit shape

- One task per commit when possible.
- Message: `engine: add ipv4 range vectors` style.
- Do not commit `*.exe`, `*.o`, or generated zips. CI builds those.

## CI fence

- PR and `master` push run tests, cppcheck, format check, both MinGW builds, both portable ZIPs, both NSIS setup EXEs, import-table gate, gitleaks, and upload an Actions artifact. Tag `v*` is the only path that creates a GitHub Release. NSIS is compiled with Ubuntu `makensis`, not npm.
- `contents: write` is allowed only on `release.yml`. CodeQL is the only job with `security-events: write`.
- Do not set `if: always()` on the artifact upload. A failed test must not publish a binary.
- Do not download a PR artifact and re-upload it as a Release asset. Release rebuilds from the tag.
- Third-party actions are pinned by commit SHA. Official `actions/upload-artifact` may use the current major (v7 as of 2026-10; v4 still accepted).
- A workflow file that shells out to an untrusted download is a failed security review.

## Stop conditions

Stop and ask the user if:

- A macOS behavior disagrees with an RFC and the plan does not already record the choice.
- A Win32 call cannot be confirmed on Windows 7.
- The binary import table shows a forbidden DLL.
- GPL-2.0 header would be removed.
