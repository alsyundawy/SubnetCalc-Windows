# Changelog — SubnetCalc-Windows

All notable changes to this project will be documented in this file.
The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.0.0] - Unreleased

### Added
- **Core Architecture**: Native ISO C11 + Win32 API implementation with zero external runtime dependencies (no .NET, no MSVC Redistributable, no UCRT requirement).
- **Target OS Compatibility**: Native support for Windows 7 SP1 (x86 and x64), Windows 8, Windows 8.1, Windows 10, and Windows 11.
- **Engine Parity**: Algorithmic baseline matching [SubnetCalc-MacOS v2.6.2](https://github.com/alsyundawy/SubnetCalc-MacOS).
- **IPv4 Engine**: Full subnet calculation, netmask, wildcard, class determination (RFC 790), RFC 3021 /31 point-to-point support, bitwise representation, and address classification (RFC 1918, loopback, link-local, multicast, reserved).
- **Cloud Subnet Profiles**: Exact reserved counts and boundary allocations for AWS VPC (5), Azure VNet (5), GCP (4), OCI (3), and Standard (2).
- **IPv6 Engine**: Full RFC 5952 text compression, prefix masking, big-integer host count (up to 2^128), nibble-reversed `ip6.arpa` PTR generation, IPv4-mapped and 6to4 conversion, RFC 4193 ULA generation via `BCryptGenRandom` (with `CryptGenRandom` fallback).
- **Subnet Planning**: FLSM (Fixed Length Subnet Mask) and VLSM (Variable Length Subnet Mask) decomposition engines with 4096-row safety limits.
- **CIDR Supernetting**: Subnet route aggregation engine.
- **Secure Export**: RFC 4180 CSV export with formula injection mitigation (CWE-1236 prefixing) and clean ASCII table export.
- **Distribution Packaging**: Dual distribution per architecture:
  - Portable single-binary ZIP (`SubnetCalc-x86.exe`, `SubnetCalc-x64.exe`) requiring zero administrator rights.
  - NSIS Installer setup (`SubnetCalc-Windows-x86-setup.exe`, `SubnetCalc-Windows-x64-setup.exe`) with clean uninstallation and Start Menu integration.
- **Automated CI/CD**: Multi-stage Linux cross-compilation via MinGW-w64, static code analysis (cppcheck, clang-format, CodeQL c-cpp, gitleaks), and automated PE import-table validation.
