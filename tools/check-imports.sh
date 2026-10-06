#!/usr/bin/env bash
set -euo pipefail

echo "==> Auditing PE binary import tables for prohibited libraries..."

mkdir -p dist
IMPORT_LOG="dist/imports.txt"
> "${IMPORT_LOG}"

FORBIDDEN_PATTERN="msvcp|vcruntime|ucrtbase|libgcc_s|libwinpthread|bcryptprimitives"
FAILED=0

for exe in dist/*.exe; do
    if [ ! -f "${exe}" ]; then
        continue
    fi
    echo "--- Auditing ${exe} ---" | tee -a "${IMPORT_LOG}"

    # Determine architecture tool
    OBJDUMP_TOOL="objdump"
    if [[ "${exe}" == *"x64"* ]] && command -v x86_64-w64-mingw32-objdump >/dev/null 2>&1; then
        OBJDUMP_TOOL="x86_64-w64-mingw32-objdump"
    elif [[ "${exe}" == *"x86"* ]] && command -v i686-w64-mingw32-objdump >/dev/null 2>&1; then
        OBJDUMP_TOOL="i686-w64-mingw32-objdump"
    fi

    # Extract DLL imports
    DUMP_OUTPUT=$(${OBJDUMP_TOOL} -p "${exe}" 2>/dev/null | grep -i "DLL Name:" || true)
    echo "${DUMP_OUTPUT}" >> "${IMPORT_LOG}"

    # Check for forbidden imports
    MATCHES=$(echo "${DUMP_OUTPUT}" | grep -Ei "${FORBIDDEN_PATTERN}" || true)
    if [ -n "${MATCHES}" ]; then
        echo "❌ ERROR: Forbidden import detected in ${exe}:" | tee -a "${IMPORT_LOG}"
        echo "${MATCHES}" | tee -a "${IMPORT_LOG}"
        FAILED=1
    else
        echo "✅ OK: Clean import table in ${exe}" | tee -a "${IMPORT_LOG}"
    fi
done

if [ ${FAILED} -ne 0 ]; then
    echo "❌ check-imports failed: prohibited dependencies found!"
    exit 1
fi

echo "✅ Import check passed successfully. All binaries are pure Win7 compliant."
