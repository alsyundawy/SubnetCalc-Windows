# Windows 7 SP1 Compatibility Specification — SubnetCalc-Windows

## 1. Operating System Baseline

- **Minimum Supported Target**: Windows 7 Service Pack 1 (SP1), x86 (32-bit) and x64 (64-bit).
- **Forward Compatibility**: Windows 8, Windows 8.1, Windows 10, Windows 11, and Windows Server 2008 R2 SP1+.

---

## 2. Zero-Dependency CRT Strategy

1. **msvcrt.dll Runtime Linkage**:
   - Stock Windows 7 SP1 installations always include `msvcrt.dll` in `C:\Windows\System32`.
   - By compiling with MinGW-w64 against `msvcrt`, SubnetCalc-Windows avoids the requirement for KB2999226 (Universal C Runtime / UCRT).
2. **Static Runtime Linking**:
   - Compiling with `-static -static-libgcc` embeds GCC runtime helper routines into the binary, eliminating external `libgcc_s_*.dll` and `libwinpthread-1.dll` dependencies.

---

## 3. Cryptography & Randomness

- **Prohibited APIs**:
  - `ProcessPrng` in `bcryptprimitives.dll` (Introduced in Windows 8 / Windows 10; causes application crashes on Windows 7).
- **Permitted APIs**:
  - `BCryptGenRandom` from `bcrypt.dll` (Available on Windows Vista / Windows 7 SP1+).
  - Fallback: `CryptGenRandom` from `advapi32.dll`.

---

## 4. High-DPI Scaling & Common Controls

- **DPI Awareness**:
  - Windows 7 SP1 supports `SetProcessDPIAware()` (available since Windows Vista).
  - Modern Windows 10/11 per-monitor v2 awareness APIs (`SetProcessDpiAwarenessContext`) are guarded and not required.
- **Common Controls**:
  - Embedded application manifest specifies Common Controls version 6.0 (`Microsoft.Windows.Common-Controls`, version `6.0.0.0`).
