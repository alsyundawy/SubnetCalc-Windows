#!/usr/bin/env bash
set -euo pipefail

MODE="${1:-test}"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${REPO_ROOT}"

mkdir -p build dist

if [ "${MODE}" = "test" ]; then
    echo "==> Running host unit tests..."
    HOST_CC="${CC:-gcc}"
    ${HOST_CC} -std=c11 -Wall -Wextra -Werror -pedantic         -I src -I src/engine         tests/test_engine.c -o build/test_engine
    ./build/test_engine
    echo "==> All host tests passed successfully."

elif [ "${MODE}" = "release" ]; then
    echo "==> Building release binaries for x86 and x64..."
    cp -f WIN7.txt dist/WIN7.txt

    COMMON_CFLAGS="-std=c11 -O2 -Wall -Wextra -Wpedantic -DWINVER=0x0601 -D_WIN32_WINNT=0x0601 -DUNICODE -D_UNICODE -municode -mwindows -static -static-libgcc"
    COMMON_LDFLAGS="-lcomctl32 -luser32 -lgdi32 -lkernel32 -lcomdlg32 -lshell32 -ladvapi32 -lbcrypt"

    # Compile Windows resource if windres exists
    if command -v i686-w64-mingw32-windres >/dev/null 2>&1 && [ -f "res/app.rc" ]; then
        i686-w64-mingw32-windres res/app.rc -O coff -o build/app_x86.res
    fi
    if command -v x86_64-w64-mingw32-windres >/dev/null 2>&1 && [ -f "res/app.rc" ]; then
        x86_64-w64-mingw32-windres res/app.rc -O coff -o build/app_x64.res
    fi

    RES_X86=""
    [ -f "build/app_x86.res" ] && RES_X86="build/app_x86.res"
    RES_X64=""
    [ -f "build/app_x64.res" ] && RES_X64="build/app_x64.res"

    # Cross-compile x86
    if command -v i686-w64-mingw32-gcc >/dev/null 2>&1; then
        echo "--> Compiling SubnetCalc-x86.exe..."
        i686-w64-mingw32-gcc ${COMMON_CFLAGS}             -I src src/main.c ${RES_X86}             -o dist/SubnetCalc-x86.exe ${COMMON_LDFLAGS}
        
        # Package portable ZIP
        echo "--> Packaging SubnetCalc-Windows-x86.zip..."
        (cd dist && zip -9 -j SubnetCalc-Windows-x86.zip SubnetCalc-x86.exe ../LICENSE WIN7.txt)
    else
        echo "⚠️ i686-w64-mingw32-gcc not found on host, skipping x86 build"
    fi

    # Cross-compile x64
    if command -v x86_64-w64-mingw32-gcc >/dev/null 2>&1; then
        echo "--> Compiling SubnetCalc-x64.exe..."
        x86_64-w64-mingw32-gcc ${COMMON_CFLAGS}             -I src src/main.c ${RES_X64}             -o dist/SubnetCalc-x64.exe ${COMMON_LDFLAGS}
        
        # Package portable ZIP
        echo "--> Packaging SubnetCalc-Windows-x64.zip..."
        (cd dist && zip -9 -j SubnetCalc-Windows-x64.zip SubnetCalc-x64.exe ../LICENSE WIN7.txt)
    else
        echo "⚠️ x86_64-w64-mingw32-gcc not found on host, skipping x64 build"
    fi

    # Build NSIS setup installers if makensis exists
    if command -v makensis >/dev/null 2>&1 && [ -f "installer/SubnetCalc.nsi" ]; then
        if [ -f "dist/SubnetCalc-x86.exe" ]; then
            echo "--> Compiling SubnetCalc-Windows-x86-setup.exe via makensis..."
            makensis -V3 -DARCH=x86 installer/SubnetCalc.nsi
        fi
        if [ -f "dist/SubnetCalc-x64.exe" ]; then
            echo "--> Compiling SubnetCalc-Windows-x64-setup.exe via makensis..."
            makensis -V3 -DARCH=x64 installer/SubnetCalc.nsi
        fi
    else
        echo "⚠️ makensis not found on host or installer/SubnetCalc.nsi missing"
    fi

    # Generate SHA256SUMS
    echo "--> Generating SHA256SUMS..."
    (
        cd dist
        SHATOOL="sha256sum"
        if ! command -v sha256sum >/dev/null 2>&1; then
            SHATOOL="shasum -a 256"
        fi
        FILES_TO_HASH=()
        for f in SubnetCalc-Windows-*.zip SubnetCalc-Windows-*-setup.exe; do
            [ -f "$f" ] && FILES_TO_HASH+=("$f")
        done
        if [ ${#FILES_TO_HASH[@]} -gt 0 ]; then
            ${SHATOOL} "${FILES_TO_HASH[@]}" > SHA256SUMS
        fi
    )

    echo "==> Release build completed."
else
    echo "Unknown mode: ${MODE}. Usage: $0 [test|release]"
    exit 1
fi
