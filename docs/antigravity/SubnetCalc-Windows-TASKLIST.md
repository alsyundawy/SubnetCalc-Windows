# SubnetCalc-Windows — Task List

Binding attachment of `SubnetCalc-Windows-ANTIGRAVITY-PROMPT.md`. Antigravity runs the master prompt, not this file.
Baseline: SubnetCalc-MacOS v2.6.2 behavior, GPL-2.0.
Executor follows `SubnetCalc-Windows-WORKGROUND.md`, builds the tree in `SubnetCalc-Windows-IMPLEMENTATION-PLAN.md`, and writes the workflows in `SubnetCalc-Windows-CI.md`.
Rule: do not mark a task done without a command and an exit code.

## Phase 0 — Repository skeleton

- [ ] T0.1 Create `alsyundawy/SubnetCalc-Windows` with GPL-2.0 `LICENSE`, README, CHANGELOG, and this plan copied into `docs/`.
- [ ] T0.2 Add `.gitignore` for `*.exe`, `*.o`, `*.d`, `build/`.
- [ ] T0.3 Add `tools/build.sh` that builds `src/engine` tests with host gcc, both MinGW targets, portable ZIPs, and both NSIS setup EXEs when `makensis` exists.
- [ ] T0.4 Add the four workflows in `SubnetCalc-Windows-CI.md`: `ci.yml` (test, lint, MinGW x86/x64, import check, portable ZIP, NSIS setup, artifact upload), `lint.yml`, `codeql.yml`, `release.yml` (tag `v*` only, GitHub Release assets).
- [ ] T0.5 README states Windows 7 SP1 x86/x64, no redistributable, credits, the macOS behavior baseline, and the Actions artifact / Release locations.
- [ ] T0.6 Add `SECURITY.md` with the report path and the forbidden-import list. Add `.gitleaks.toml` allowlist only for known test vectors, not for real secrets.

Done when: empty `wWinMain` EXE builds for both arches, both portable ZIPs and both NSIS setup EXEs exist, the import check passes, and a push to `master` publishes the Actions artifact. Do not mark T0.4 done if the artifact step or the setup EXE is missing.

## Phase 1 — IPv4 engine

- [ ] T1.1 Write `tests/vectors.txt` rows for `192.168.1.10/24`, `10.0.0.0/8`, `172.16.0.0/12`, `0.0.0.0/0`, `255.255.255.255/32` before coding.
- [ ] T1.2 `ipv4_parse`, `ipv4_format`, contiguous-mask check. Reject `255.255.0.255`.
- [ ] T1.3 Network, broadcast, wildcard, class, class bits, subnet bits, host bits.
- [ ] T1.4 Host count helper that does not shift by 32. Document `/31` = 2 and the `/32` decision in `docs/rfc-notes.md`.
- [ ] T1.5 Binary, hex, and `n/s/h` bitmap strings.
- [ ] T1.6 Classifier: RFC 1918, loopback, link-local, multicast, reserved. Add CGNAT only if the macOS function contains `100.64.0.0/10`.

Done when: `tests/test_ipv4` exits 0.

## Phase 2 — IPv6 engine

- [ ] T2.1 Vectors for `2001:db8::1/64`, `::1/128`, `fe80::1/10`, `::ffff:192.0.2.1`, `2001:db8:0:0:1::/64` compression.
- [ ] T2.2 Parse with a single `::`, reject two `::`, reject more than 8 groups.
- [ ] T2.3 RFC 5952 compact form: lowercase, longest zero run, no compression of a single zero group.
- [ ] T2.4 Network mask, range, `ip6.arpa` nibble reverse.
- [ ] T2.5 Big-integer decimal for `2^(128-prefix)`.
- [ ] T2.6 IPv4-mapped and 6to4 conversion, matching the macOS function names' behavior.
- [ ] T2.7 ULA: `fd` prefix plus 40 bits from `BCryptGenRandom`, `CryptGenRandom` fallback. Test only the shape, not the random value.

Done when: `tests/test_ipv6` exits 0.

## Phase 3 — Cloud, FLSM, VLSM, CIDR, export

- [ ] T3.1 Port `CloudProfile` numbers exactly: AWS 5, Azure 5, GCP 4, OCI 3, standard 2, with the minimum prefixes 28/29/29/30/32.
- [ ] T3.2 Vectors: AWS and Azure `/24` usable count 251; GCP 252; OCI 253.
- [ ] T3.3 FLSM: split a base prefix into N equal children. Reject a split that does not fit.
- [ ] T3.4 VLSM: sort requirements descending, allocate aligned blocks, report waste and efficiency. Cap 4096 rows.
- [ ] T3.5 CIDR aggregation: merge adjacent aligned blocks. Vectors written before code.
- [ ] T3.6 CSV RFC 4180 quoting plus formula-injection prefix. ASCII table writer.

Done when: `tests/test_cloud` and `tests/test_export` exit 0.

## Phase 4 — Shell UI

- [ ] T4.1 Manifest, icon from the macOS PNG set converted to ICO, `wWinMain`, common-controls init, DPI aware.
- [ ] T4.2 Main window, menu, status bar, six tabs. No calculation yet.
- [ ] T4.3 About dialog with GPL-2.0 and credits.
- [ ] T4.4 Keyboard: Ctrl+C copies the focused result, Ctrl+E exports, Alt+F4 exits.

Done when: both EXEs open on a Windows 7 VM or, if no VM is available, the task note says "UI not runtime-verified" and does not claim otherwise.

## Phase 5 — Tab wiring

- [ ] T5.1 IPv4 tab: combo, slider, live fields, badge, bitmap.
- [ ] T5.2 Subnets/Hosts table and copy.
- [ ] T5.3 CIDR tab wired to `cidr.c`.
- [ ] T5.4 FLSM tab wired to `split.c`.
- [ ] T5.5 VLSM tab: editable requirement list, results, efficiency.
- [ ] T5.6 IPv6 tab: prefix slider, mapped/6to4, ULA button, `ip6.arpa`.
- [ ] T5.7 Export dialogs for CSV and ASCII on the active table.
- [ ] T5.8 History file in `%APPDATA%\SubnetCalc\history.txt`, max 100, Clear History.

Done when: each tab's manual script in `docs/manual-check.md` is filled with observed values, not planned values.

## Phase 6 — Themes and polish

- [ ] T6.1 Six palettes: Classic, Dark, Catppuccin Mocha, Dracula, Tokyo Night, High Contrast. Persist the choice.
- [ ] T6.2 Owner-draw badge colors. Do not block v1 on the other 19 macOS families.
- [ ] T6.3 High-contrast palette must meet readable text/background pairs. No contrast-ratio claim without a measured pair.

## Phase 7 — Release

- [ ] T7.1 `release.yml` builds both portable ZIPs and both NSIS setup EXEs, writes `SHA256SUMS`, and uploads them to a GitHub Release. Do not attach a PR artifact as the release binary.
- [ ] T7.2 Release body includes the import-table result and the unsigned-EXE / SmartScreen warning for both the portable EXE and the setup EXE (`WIN7.txt`).
- [ ] T7.3 CodeQL workflow has completed at least once on `master` without a Critical or High alert left unexplained.
- [ ] T7.4 Tag `v1.0.0` only after Phase 1–5 tests are green, `ci.yml` is green, and the Phase 4 runtime note is honest.

## Explicitly later

- Windows ARM64 build.
- Authenticode.
- Remaining 19 themes.
- Print view (macOS `PrintView.m` is out of scope for v1).
- SQLite history.
