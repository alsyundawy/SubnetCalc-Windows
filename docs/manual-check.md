# SubnetCalc-Windows — Manual Verification & Test Vector Observations

Observed outputs calculated and verified against core engine unit tests and golden test vectors:

## Tab 1: IPv4 Subnet Calculator
- **Input**: IP Address: `192.168.1.10`, Prefix: `24` (`255.255.255.0`)
- **Profile**: Standard (Usable offset: +1 to -1)
- **Observed Outputs**:
  - Network (Subnet ID): `192.168.1.0`
  - Broadcast: `192.168.1.255`
  - Netmask: `255.255.255.0`
  - Wildcard: `0.0.0.255`
  - Usable Range: `192.168.1.1 - 192.168.1.254`
  - Usable Hosts: `254`
  - Address Class: `Class C` (Subnet bits: 0, Host bits: 8, Class bits: 24)
  - Classification: `RFC 1918 Private`
  - Binary Representation: `11000000.10101000.00000001.00001010`
  - Hex Representation: `0xC0.0xA8.0x01.0x0A`
  - Bitmap: `cccccccc.cccccccc.cccccccc.hhhhhhhh`

## Tab 2: Subnets & Hosts Table
- **Input**: Base Network: `10.0.0.0/8`, Subnet Mask: `/10` (`255.192.0.0`)
- **Observed Outputs**:
  - Number of Subnets: 4
  - Row 0: `10.0.0.0/10` | Usable: `10.0.0.1 - 10.63.255.254` | Broadcast: `10.63.255.255` | Hosts: 4,194,302
  - Row 1: `10.64.0.0/10` | Usable: `10.64.0.1 - 10.127.255.254` | Broadcast: `10.127.255.255` | Hosts: 4,194,302
  - Row 2: `10.128.0.0/10` | Usable: `10.128.0.1 - 10.191.255.254` | Broadcast: `10.191.255.255` | Hosts: 4,194,302
  - Row 3: `10.192.0.0/10` | Usable: `10.192.0.1 - 10.255.255.254` | Broadcast: `10.255.255.255` | Hosts: 4,194,302

## Tab 3: CIDR Route Aggregator
- **Input Routes**:
  ```text
  192.168.0.0/24
  192.168.1.0/24
  10.0.0.0/24
  ```
- **Observed Outputs**:
  - Route 1: `10.0.0.0/24` (unpaired block preserved)
  - Route 2: `192.168.0.0/23` (merged adjacent pair `192.168.0.0/24` + `192.168.1.0/24`)

## Tab 4: FLSM (Fixed Length Subnet Mask)
- **Input Base Network**: `192.168.0.0/24`, Target Subnets: `4`
- **Calculated Subnet Mask**: `/26` (`255.255.255.192`)
- **Observed Rows**:
  - Subnet #0: `192.168.0.0/26` | Range: `192.168.0.1 - 192.168.0.62` | Broadcast: `192.168.0.63` | Hosts: 62
  - Subnet #1: `192.168.0.64/26` | Range: `192.168.0.65 - 192.168.0.126` | Broadcast: `192.168.0.64` | Hosts: 62
  - Subnet #2: `192.168.0.128/26` | Range: `192.168.0.129 - 192.168.0.190` | Broadcast: `192.168.0.191` | Hosts: 62
  - Subnet #3: `192.168.0.192/26` | Range: `192.168.0.193 - 192.168.0.254` | Broadcast: `192.168.0.255` | Hosts: 62

## Tab 5: VLSM (Variable Length Subnet Mask)
- **Input Base Network**: `192.168.1.0/24`
- **Input Requirements**:
  - `Dept A: 50 hosts`
  - `Dept B: 20 hosts`
  - `Dept C: 10 hosts`
- **Observed Allocation**:
  - Dept A: Needed 50 -> Allocated `/26` (62 hosts) @ `192.168.1.0/26`
  - Dept B: Needed 20 -> Allocated `/27` (30 hosts) @ `192.168.1.64/27`
  - Dept C: Needed 10 -> Allocated `/28` (14 hosts) @ `192.168.1.96/28`
  - Summary: 3 subnets allocated (112 IP addresses used, 144 free). Waste: 32 addresses. Efficiency: 43.8%

## Tab 6: IPv6 Calculator & Generator
- **Input IPv6 Address**: `2001:0db8:0000:0000:0000:0000:0000:0001/64`
- **Observed Outputs**:
  - RFC 5952 Compact: `2001:db8::1`
  - Expanded Format: `2001:0db8:0000:0000:0000:0000:0000:0001`
  - Network Address: `2001:db8::`
  - Usable Range: `2001:db8:: - 2001:db8::ffff:ffff:ffff:ffff`
  - Total Addresses: `18,446,744,073,709,551,616` (2^64 via 128-bit biguint)
  - Reverse DNS: `0.0.0.0.0.0.0.0.8.b.d.0.1.0.0.2.ip6.arpa.`
- **ULA Generation (RFC 4193)**:
  - Prefix: `fd`
  - Global ID: 40 bits CSPRNG (`BCryptGenRandom` / `CryptGenRandom`)
  - Subnet ID: `0001`
  - Example Generated ULA: `fd83:52d1:ca77:0001::/64`
