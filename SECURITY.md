# Security Policy — SubnetCalc-Windows

## Supported Versions

| Version | Supported |
| :--- | :--- |
| 1.0.x | Yes |
| < 1.0.0 | No |

## Reporting a Vulnerability

If you discover a security vulnerability in SubnetCalc-Windows, please report it responsibly:

1. **Email**: Contact Harry Dertin Sutisna Alsyundawy directly at `security@alsyundawy.com` or `alsyundawy@gmail.com`.
2. **GitHub Security Advisories**: You can also report via private vulnerability reporting on the GitHub repository at [Security Advisories](https://github.com/alsyundawy/SubnetCalc-Windows/security/advisories).

Please provide a detailed description of the vulnerability, reproduction steps, and potential impact. Reports will be acknowledged within 48 hours, and fixes will be coordinated before public disclosure.

## Security Architecture & Invariants

SubnetCalc-Windows enforces strict runtime and memory safety invariants:

1. **Zero Prohibited Import Policy**:
   Binaries are audited with `tools/check-imports.sh` during CI to ensure no dependencies on:
   - `msvcp*.dll` (No C++ runtime)
   - `vcruntime*.dll` (No VC runtime redistributables)
   - `ucrtbase.dll` (No UCRT requirement on Windows 7 SP1)
   - `libgcc_s_*.dll` (Fully statically linked GCC runtime)
   - `libwinpthread-*.dll` (Zero pthread runtime DLLs)
   - `bcryptprimitives.dll` / `ProcessPrng` (Not present on Windows 7; strictly avoided)

2. **CWE-1236 (Formula Injection Defense)**:
   All CSV exports sanitize cells beginning with dangerous characters (`=`, `+`, `-`, `@`, `|`, tab) by prepending a single quote (`'`).

3. **CWE-330 (Cryptographically Strong Entropy)**:
   IPv6 Unique Local Address (RFC 4193) 40-bit Global IDs are generated exclusively using `BCryptGenRandom` (with fallback to `CryptGenRandom`). Predictable pseudo-random number generators (`rand()`, `srand()`) are strictly forbidden.

4. **CWE-190 & CWE-770 (Safe Bitwise & Bounded Allocation)**:
   All bit-shift operations guard against undefined behavior (e.g., shifting by ≥ 32 or ≥ 64). FLSM and VLSM table row generations are capped at 4096 rows. History is strictly capped at 100 entries.
