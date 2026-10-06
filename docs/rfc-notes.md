# RFC Implementation Decisions & Golden Vectors — SubnetCalc-Windows

This document records algorithmic decisions and RFC citations implemented in SubnetCalc-Windows, maintaining functional parity with [SubnetCalc-MacOS v2.6.2](https://github.com/alsyundawy/SubnetCalc-MacOS).

---

## 1. RFC 3021: Using 31-Bit Prefixes on IPv4 Point-to-Point Links

- **Prefix /31**:
  - In traditional IPv4 subnetting (RFC 790/950), a subnet reserves the lowest address for the network and the highest for broadcast.
  - Per **RFC 3021**, on dedicated point-to-point links, both addresses in a `/31` subnet are assigned to interfaces (host count = 2, usable host range encompasses both addresses).
  - SubnetCalc-Windows treats `/31` networks as containing 2 usable host addresses, matching SubnetCalc-MacOS.

---

## 2. IPv4 /32 Single Host Prefix Decision

- **Discrepancy in macOS Upstream**:
  - In `IPSubnetcalc.swift` of SubnetCalc-MacOS v2.6.2, `maxHosts()` for prefix `/32` returns `0`.
  - Conversely, `CloudProfile.standard` calculates usable host range count as `1` for prefix `/32`.
- **Adopted Specification in SubnetCalc-Windows**:
  - A `/32` prefix represents an individual host address (host bits = 0).
  - Usable range: The single IP address itself (`host - host`).
  - Max usable host count: Reported as `1` in host calculation summaries, with 0 unassigned host bits.
  - Documented in test vectors (`tests/vectors.txt`).

---

## 3. RFC 4193: Unique Local IPv6 Unicast Addresses (ULA)

- Prefix: `fc00::/7`.
- Local bit (L) is set to `1` (yielding prefix `fd00::/8`).
- 40-bit Global ID is generated via a cryptographically secure pseudo-random number generator (CSPRNG):
  - Primary API: `BCryptGenRandom` with `BCRYPT_USE_SYSTEM_PREFERRED_RNG`.
  - Secondary Fallback: `CryptGenRandom` from `advapi32.dll`.
- Test vectors verify formatting and `fd` prefix without asserting fixed random payloads.

---

## 4. RFC 5952: Recommendation for IPv6 Address Text Representation

- Lowercase hexadecimal characters `[a-f0-9]`.
- Leading zeros in each 16-bit field are omitted (e.g., `0001` becomes `1`, `0000` becomes `0`).
- The longest run of consecutive 16-bit zero fields is compressed using `::`.
- A single field of `0` is never shortened to `::` (e.g., `2001:db8:0:1::` instead of `2001:db8::1::`).
- When two zero sequences are of equal length, the first occurrence is compressed.

---

## 5. Cloud Subnet Provider Specifications

Reserved host allocations ported from `CloudProfile` in `IPSubnetcalc.swift`:

| Profile | Reserved Count | Reserved Offsets / Roles | Usable Start | Usable End | Min Prefix |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Standard** | 2 (if prefix ≤ 30) | Network (.0), Broadcast (.255) | `network + 1` | `broadcast - 1` | /32 |
| **AWS VPC** | 5 | .0 (Net), .1 (Router), .2 (DNS), .3 (Future), .255 (Bcast) | `network + 4` | `broadcast - 1` | /28 |
| **Azure VNet** | 5 | .0 (Net), .1 (DefGW), .2 (DNS), .3 (DNS), .255 (Bcast) | `network + 4` | `broadcast - 1` | /29 |
| **GCP** | 4 | .0 (Net), .1 (Gateway), .254 (Reserved), .255 (Bcast) | `network + 2` | `broadcast - 2` | /29 |
| **OCI** | 3 | .0 (Net), .1 (DefGW), .255 (Bcast) | `network + 2` | `broadcast - 1` | /30 |
