#include "about.h"

void show_about_dialog(HWND hParent) {
    const wchar_t *about_text =
        L"SubnetCalc for Windows — v1.0.0\n"
        L"High-Performance Native C11 & Win32 IPv4/IPv6 Subnet Calculator\n\n"
        L"Author & Upstream Attribution:\n"
        L"• Original Author & Creator: Julien Mulot (mulot/SubnetCalc)\n"
        L"  https://subnetcalc.mulot.org\n\n"
        L"• Ported, Architected & Maintained by:\n"
        L"  Harry Dertin Sutisna Alsyundawy (@alsyundawy)\n"
        L"  ALSYUNDAWY IT SOLUTION\n\n"
        L"Algorithmic Baseline:\n"
        L"• SubnetCalc-MacOS v2.6.2 (100% Mathematical Parity)\n\n"
        L"Platform & System Requirements:\n"
        L"• Windows 7 SP1 x86 & x64 through Windows 11\n"
        L"• Zero Runtime Dependencies (No .NET, No MSVC Redistributable, No UCRT)\n\n"
        L"License: GNU General Public License v2 (GPL-2.0)";

    MessageBoxW(hParent, about_text, L"About SubnetCalc", MB_OK | MB_ICONINFORMATION);
}
