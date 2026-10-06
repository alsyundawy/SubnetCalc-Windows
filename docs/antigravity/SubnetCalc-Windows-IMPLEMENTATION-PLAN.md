# SubnetCalc-Windows — Implementation Plan

Binding attachment of `SubnetCalc-Windows-ANTIGRAVITY-PROMPT.md`. Antigravity runs the master prompt, not this file.
Related: `SubnetCalc-Windows-WORKGROUND.md` (rules), `SubnetCalc-Windows-CI.md` (workflows), `SubnetCalc-Windows-TASKLIST.md` (task ids).
Status: planning artifact for Antigravity IDE
Target repository: `https://github.com/alsyundawy/SubnetCalc-Windows`
Source of truth for behavior: `https://github.com/alsyundawy/SubnetCalc-MacOS` at `master`, version v2.6.2 (2026-10-06)
License of the source: GPL-2.0 (`License.txt`). This port is a derivative work and must stay GPL-2.0.
Language of this document: English
Date: 2026-10-06

## 1. Goal

Build a native Windows subnet calculator that preserves the calculation behavior of SubnetCalc-MacOS v2.6.2 and runs on Windows 7 SP1, both 32-bit and 64-bit, with no installed runtime.

Hard constraints:

- Minimum OS: Windows 7 SP1 x86 and x64. Windows 8, 8.1, 10, and 11 must also run the same binaries.
- No .NET Framework, no .NET (Core), no Visual C++ Redistributable, no UCRT redistributable, no Electron, no WebView.
- Offline. No telemetry. No network call on the calculation path.
- Two deliverables per architecture: a portable ZIP (single EXE, no admin) and an NSIS installer (Start Menu, uninstaller, Add/Remove Programs).
- GPL-2.0. Keep Julien Mulot and Harry Dertin Sutisna Alsyundawy credits.

Non-goals for v1:

- Windows on ARM64 (optional later; not required by the Win7 constraint).
- MSI and Inno Setup. The installer is NSIS only.
- Authenticode signing. Document SmartScreen warning on both the portable EXE and the setup EXE.
- Pixel-perfect clone of the AppKit theme engine. Functional parity first, then a reduced theme set.

## 2. What the macOS app actually is

Verified from the public repository on 2026-10-06. Not inferred.

Repository: `alsyundawy/SubnetCalc-MacOS`
Upstream: `mulot/SubnetCalc` (Julien Mulot, subnetcalc.mulot.org)
Sibling: `alsyundawy/SubNetCalc-Electron`
Current version: v2.6.2
License: GPL-2.0
UI: Swift / AppKit. Six tabs: IPv4, Subnets/Hosts, CIDR, FLSM, VLSM, IPv6.
Engine file: `IPSubnetcalc.swift` (about 58 KB)
UI controller: `SubnetCalcAppDelegate.swift` (about 94 KB)
Themes: `ThemeManager.swift` (about 58 KB)
History: Core Data entity `AddrHistory`, LRU
Print: leftover Objective-C `PrintView.m`

Calculation outputs that must match:

- IPv4: subnet id, broadcast, dotted mask, wildcard, class (RFC 790), class bits, subnet bits, host bits, binary map, hex map, n/s/h bitmap, usable range, max hosts.
- Prefix rules already in source:
  - `/31`: 2 usable hosts (RFC 3021). Both ends are usable.
  - `/32` in `maxHosts()`: returns 0.
  - `CloudProfile.standard` usable range for `/32`: count 1. This disagrees with `maxHosts()`. Do not silently pick one. Add a golden test and record the chosen rule in `docs/rfc-notes.md`.
- IPv6: parse and validate, expand, RFC 5952 compact form, network address, range, address count as a big integer, binary, hex ID, `ip6.arpa`, IPv4-mapped (`::ffff:0:0/96`), 6to4, RFC 4193 ULA with 40-bit CSPRNG Global ID, reserved-block lookup.
- Classification present in source: IPv4 RFC 1918, loopback `127.0.0.0/8`, link-local `169.254.0.0/16`, multicast `224.0.0.0/4`, reserved `240.0.0.0/4`. README also claims CGNAT. Extract the full `classifyIPv4Address` body before coding the badge. Do not invent a CGNAT branch if the function does not have `100.64.0.0/10`.
- Cloud profiles, copied from `CloudProfile` in `IPSubnetcalc.swift`:

| Profile | Reserved | Usable start | Usable end | Minimum prefix accepted |
| --- | --- | --- | --- | --- |
| Standard | 2 if prefix <= 30, else 0 | network+1, except /31 and /32 | broadcast-1, except /31 and /32 | 32 |
| AWS VPC | 5 | network+4 | broadcast-1 | 28 |
| Azure VNet | 5 | network+4 | broadcast-1 | 29 |
| GCP | 4 | network+2 | broadcast-2 | 29 |
| OCI | 3 | network+2 | broadcast-1 | 30 |

If prefix is longer than `minimumPrefix`, `reservedCount` returns 0 and `usableRange` returns nil. Match that.

- Export: RFC 4180 CSV with formula-injection defense (prefix `'`, CWE-1236) and an ASCII table.
- Persistence: calculation history with LRU and Clear History.
- Themes: 25 families on macOS. Windows v1 ships 6 palettes. The rest are a later task.

## 3. Stack decision

Chosen stack: ISO C11 + Win32 API, cross-compiled with MinGW-w64 against `msvcrt.dll`.

Why this stack:

- `msvcrt.dll` is already on Windows 7. No redistributable.
- Static `-static -static-libgcc` removes `libgcc_s_*.dll` and `libwinpthread-1.dll`.
- Win32 (`user32`, `gdi32`, `comctl32`, `comdlg32`, `shell32`, `advapi32`, `bcrypt`) is present on Windows 7.
- C is enough for bitwise subnet math. No C++ ABI, no RTTI, no exception tables.

Rejected stacks, with reasons:

| Stack | Why rejected |
| --- | --- |
| .NET / WinForms / WPF / WinUI | User constraint. Also WinUI does not run on Windows 7. |
| MSVC `/MD` | Requires VC++ Redistributable. Forbidden. |
| MSVC `/MT` | Legal technically (CRT is inside the EXE), but the public CI story is worse and it is still the Visual CRT. Not the default. |
| MinGW UCRT64 | Stock Windows 7 does not have UCRT until KB2999226. Forbidden. |
| Qt / wxWidgets / GTK | DLL or huge static binary, extra license surface. |
| Electron | Sibling repo already exists. Modern Electron does not support Windows 7. |
| Rust 1.78+ | Upstream last Windows 7 toolchain is 1.77.2. Frozen toolchain is a maintenance trap. |
| Go 1.21+ | Upstream dropped Windows 7. A fork exists (`thongtech/go-legacy-win7`) but is not the official toolchain. |
| Python, Java | Runtime. Forbidden. |

Allowed system imports only:

- `kernel32.dll`, `user32.dll`, `gdi32.dll`, `comctl32.dll`, `comdlg32.dll`, `shell32.dll`, `advapi32.dll`, `bcrypt.dll`
- No `bcryptprimitives.dll` (`ProcessPrng` is Windows 8+ and aborts on Windows 7).
- ULA entropy: `BCryptGenRandom` with `BCRYPT_USE_SYSTEM_PREFERRED_RNG`. Fallback: `CryptGenRandom` from `advapi32.dll`.

Compiler flags, both architectures:

```
-std=c11 -O2 -Wall -Wextra -Wpedantic -Wformat=2 -Wconversion -DWINVER=0x0601 -D_WIN32_WINNT=0x0601 -DUNICODE -D_UNICODE -municode -mwindows -static -static-libgcc
```

Targets:

- `i686-w64-mingw32-gcc` → `SubnetCalc-x86.exe`
- `x86_64-w64-mingw32-gcc` → `SubnetCalc-x64.exe`

Manifest must declare:

- Common Controls v6
- DPI aware (`SetProcessDPIAware`, which exists on Vista+)
- Supported OS: Windows 7, 8, 8.1, 10, 11
- Execution level `asInvoker`
- UTF-8 active code page is optional and must not be required for Windows 7. Use wide Win32 APIs (`*W`) and convert at the boundary.

Do not call: `SetProcessDpiAwarenessContext`, `ProcessPrng`, any WinRT API, `TaskDialogIndirect` without a comctl32 v6 fallback path tested on Windows 7.

## 4. Architecture

```
SubnetCalc-Windows/
  LICENSE                 GPL-2.0 text
  README.md
  CHANGELOG.md
  docs/
    implementation-plan.md
    rfc-notes.md
    win7-compat.md
  src/
    main.c                wWinMain, message loop, accelerators
    app.h
    engine/
      ipv4.h ipv4.c       parse, mask, range, class, bitmap
      ipv6.h ipv6.c       8 x uint16 words, RFC 5952, ip6.arpa
      classify.h classify.c
      cloud.h cloud.c     exact CloudProfile table
      split.h split.c     FLSM and VLSM
      cidr.h cidr.c       supernet / aggregation
      export.h export.c   CSV and ASCII
      ula.h ula.c         RFC 4193, BCryptGenRandom
      biguint.h biguint.c 128-bit address count, decimal print
    ui/
      window.c            main frame, status bar
      tabs.c              6 tabs via SysTabControl32
      tab_ipv4.c
      tab_hosts.c
      tab_cidr.c
      tab_flsm.c
      tab_vlsm.c
      tab_ipv6.c
      about.c
      theme.c             6 palettes, owner-draw where needed
      clipboard.c
    persist/
      history.c           LRU file under %APPDATA%\SubnetCalc\history.txt
    res/
      app.rc app.manifest icons
  tests/
    test_ipv4.c
    test_ipv6.c
    test_cloud.c
    test_export.c
    vectors.txt
  tools/
    build.sh              cross build from Linux, portable ZIP, makensis
    build.ps1             native MinGW build on Windows
    check-imports.sh
  installer/
    SubnetCalc.nsi        NSIS, compiled twice (/DARCH=x86 and /DARCH=x64)
  .github/workflows/build.yml
```

Module rule: the engine has zero Win32 includes. UI calls the engine. Tests link only the engine. This is the portability and review boundary.

Data model:

- IPv4 address and mask: `uint32_t` host order. Convert at parse/print only.
- IPv6 address: `uint16_t word[8]`, network order inside each word, plus `int prefix` in `0..128`.
- Host counts above 32 bits: small big-integer (4 x `uint32_t` is enough for 2^128). Never use `double`.
- VLSM rows: heap array with a hard cap (4096 rows). Reject above the cap. No unbounded `realloc` loop.
- History: append-only text, max 100 entries, LRU by move-to-front. No SQLite in v1. SQLite amalgamation is allowed later if the text format proves insufficient.

UI layout, 960x640 default, resizable, min 800x560:

1. IPv4 — address combo, mask combo, slider 0..32, result fields, class badge, binary/hex/bitmap.
2. Subnets/Hosts — generated table, copy, CSV, ASCII.
3. CIDR — list of networks in, aggregated prefixes out.
4. FLSM — base network, subnet count or new prefix, table.
5. VLSM — base network, host-requirement rows, ordered allocation, efficiency.
6. IPv6 — address, prefix slider 0..128, mapped/6to4, ULA button, `ip6.arpa`, classification.

Menu: File (Export CSV, Export ASCII, Exit), Edit (Copy result, Clear history), View (theme), Help (About, RFC notes).

## 5. Parity rules

Golden vectors, minimum set, checked into `tests/vectors.txt`:

- `192.168.1.10/24` → network `192.168.1.0`, broadcast `192.168.1.255`, mask `255.255.255.0`, wildcard `0.0.0.255`, usable `192.168.1.1 - 192.168.1.254`, hosts 254.
- `10.0.0.1/8` class A. `172.16.5.4/12` private. `192.168.0.1/16` private.
- `10.1.2.3/31` → 2 hosts, usable both addresses (RFC 3021).
- `10.1.2.3/32` → documented chosen behavior (see disagreement above).
- AWS `10.0.0.0/24` → usable `10.0.0.4 - 10.0.0.254`, count 251. Same numbers for Azure.
- GCP `10.0.0.0/24` → usable `10.0.0.2 - 10.0.0.253`, count 252.
- OCI `10.0.0.0/24` → usable `10.0.0.2 - 10.0.0.254`, count 253.
- `2001:db8::1/64` network `2001:db8::/64`, compressed per RFC 5952 (lowercase hex).
- `::ffff:192.0.2.1` classified IPv4-mapped.
- `ip6.arpa` for `2001:db8::1` ends in `ip6.arpa` and is nibble-reversed.
- CSV cell `=cmd|` becomes `'=cmd|`.
- ULA starts with `fd` (locally assigned, RFC 4193 section 3.2.1 L bit = 1). Global ID is 40 random bits. Never a constant.

Aggregation examples must be written before the CIDR tab is implemented. Do not invent expected output in the test after the code exists.

## 6. Security and production constraints

Mapped to real weakness classes, not guessed CVEs:

- CWE-1236: CSV formula injection. Keep the macOS sanitizer behavior.
- CWE-22: export dialog must use the common-item dialog and must not concatenate an unsanitized path.
- CWE-190: prefix math must use widening multiply/shift. `1u << (32 - prefix)` is undefined at prefix 0 if coded naively. Use a checked helper.
- CWE-770: VLSM/FLSM row cap and history cap.
- CWE-330: ULA must use `BCryptGenRandom` or `CryptGenRandom`, never `rand()`.
- CWE-252: check every parse and every Win32 return that affects a path or file write.

Review gate before tag: the 13 pillars in `WORKGROUND.md`. No release if Critical or High remains open.

## 7. Build, CI, release

Mirror the macOS repo split (`build.yml`, `release.yml`, `codeql.yml`, linter workflow). Do not put release publishing in the PR workflow. Full job design is in `SubnetCalc-Windows-CI.md`. Task ids are T0.4 and T7.1 in `SubnetCalc-Windows-TASKLIST.md`.

Four workflows, all on `ubuntu-latest` unless noted:

| Workflow | Trigger | What it publishes |
| --- | --- | --- |
| `.github/workflows/ci.yml` | pull_request, push to `master`, workflow_dispatch | Actions artifact `subnetcalc-windows-build` (14-day retention): both EXE, both ZIP, `SHA256SUMS`, `imports.txt` |
| `.github/workflows/lint.yml` | pull_request, push to `master` | No binary. Fails the check on cppcheck error or format drift |
| `.github/workflows/codeql.yml` | pull_request, push to `master`, weekly schedule | GitHub code scanning alerts. No release asset |
| `.github/workflows/release.yml` | push tag `v*` | GitHub Release assets only. Does not run on PR |

CI job order, one workflow so a red test cannot upload a green-looking artifact:

1. `test` — host `gcc -std=c11 -Wall -Wextra -Werror` on `src/engine` plus `tests/`. Exit non-zero fails the workflow.
2. `lint` — `cppcheck --error-exitcode=2 --enable=warning,style,performance,portability` on `src/`. Format check with `clang-format --dry-run -Werror` if `.clang-format` exists.
3. `build` — `gcc-mingw-w64` for `i686-w64-mingw32` and `x86_64-w64-mingw32`. Package ZIP. `objdump -p` must not show `msvcp`, `vcruntime`, `ucrtbase`, `libgcc_s`, `libwinpthread`, `bcryptprimitives`.
4. `upload` — `actions/upload-artifact` (v7 is current as of 2026-10; v4 still works). `if: always()` is forbidden. Upload only when build succeeded.
5. `security-scan` — `gitleaks` on the checkout. Fail on a detected secret. This is not a substitute for CodeQL.

Release job, tag `v*.*.*` only:

1. Re-run test, lint, both cross builds, import check, then `makensis` for both architectures. Do not reuse a PR artifact as the release binary.
2. Write `SHA256SUMS` with `sha256sum` over the portable ZIPs and the setup EXEs.
3. `gh release create` with `contents: write` on that job only. Assets listed in `SubnetCalc-Windows-CI.md`.
4. Release notes must say both the portable EXE and the setup EXE are unsigned and SmartScreen may warn. No Authenticode in v1.

NSIS, required in v1:

- Script: `installer/SubnetCalc.nsi`, compiled twice (`/DARCH=x86` and `/DARCH=x64`) with the Ubuntu `nsis` package (`makensis`). Do not pull NSIS from npm.
- `Unicode True`. `RequestExecutionLevel admin` because the installer writes Program Files. Portable ZIP stays the no-admin path.
- `ManifestSupportedOS` for Windows 7, 8, 8.1, and 10. No Win10-only plugin.
- Install dir: `$PROGRAMFILES32` for x86, `$PROGRAMFILES64` for x64. Start Menu shortcut. Uninstaller registered in Add/Remove Programs. No desktop shortcut by default.
- Payload is only the matching portable EXE plus `LICENSE` and `WIN7.txt`. No .NET and no VC++ redistributable inside the installer.
- Setup names: `SubnetCalc-Windows-x86-setup.exe`, `SubnetCalc-Windows-x64-setup.exe`.

Permissions:

- `ci.yml`, `lint.yml`: `contents: read`.
- `codeql.yml`: `contents: read`, `security-events: write`.
- `release.yml`: `contents: write` only. No pull_request trigger.

CodeQL: `github/codeql-action` v3 or v4 (both supported per the CodeQL Action README). Language `c-cpp`. `build-mode: manual`, then compile the engine tests with host gcc so the database traces real code. Do not set `build-mode: none` for this C tree.

Pin third-party actions by commit SHA when added (`gitleaks/gitleaks-action`). Official `actions/*` and `github/codeql-action` may use a major tag, recorded in `docs/ci.md`.

Versioning starts at `1.0.0` for the Windows tree. README must point at macOS v2.6.2 as the behavior baseline, not pretend the Windows tree is v2.6.2.

## 8. Audit note on the macOS tree

Scope inspected: public README, DOCNOTE, CHANGELOG, GitHub contents API, and the `CloudProfile` / `maxHosts` / classifier portions of `IPSubnetcalc.swift`. Full line audit of `SubnetCalcAppDelegate.swift` was not done in this planning pass. Do not claim the macOS tree is clean.

Findings that affect the port:

- Maintainability: three files over 50 KB (`SubnetCalcAppDelegate.swift`, `IPSubnetcalc.swift`, `ThemeManager.swift`). The Windows tree must not copy that shape.
- Logic: `/32` host count disagrees between `maxHosts()` (0) and `CloudProfile.standard` (1). Resolve in writing.
- Security already present and worth porting: CSV formula sanitizer.
- License: GPL-2.0. Do not relicense.

## 9. Delivery order

See `SubnetCalc-Windows-TASKLIST.md`. Engine and golden tests land before any window. A tab is not done until its vectors pass. CI from `SubnetCalc-Windows-CI.md` lands in Phase 0, before feature work. The session that executes this plan is started only from `SubnetCalc-Windows-ANTIGRAVITY-PROMPT.md`.
