# SubnetCalc-Windows — CI, artifacts, release, lint, security

Binding attachment of `SubnetCalc-Windows-ANTIGRAVITY-PROMPT.md`. Antigravity runs the master prompt, not this file.
Implements section 7 of `SubnetCalc-Windows-IMPLEMENTATION-PLAN.md`. Task ids T0.4, T0.6, T7.1–T7.3 are in `SubnetCalc-Windows-TASKLIST.md`. Rules for permissions and artifact upload are in `SubnetCalc-Windows-WORKGROUND.md`.
Pattern follows `alsyundawy/SubnetCalc-MacOS`: separate build, release, CodeQL, and linter workflows (that repo has `build.yml`, `release.yml`, `codeql.yml`, `mega-linter.yml`, `super-linter.yml`).

Verified action facts used here (2026-10-06):

- `github/codeql-action` v3 and v4 are the supported majors. Language for this tree is `c-cpp`.
- `actions/upload-artifact` v7 exists (v7.0.0 / v7.0.1). v4 still works. Pin the major you actually tested.
- Third-party actions (`gitleaks/gitleaks-action`) must be pinned by commit SHA, not a floating tag.

## What the user sees on GitHub

- Pull request checks: `test`, `lint`, `build`, `import-check`, `gitleaks`, `codeql`.
- Actions artifact on push to `master` and on pull requests: `subnetcalc-windows-build`, retention 14 days.
- Release page, only after `git tag v1.0.0 && git push origin v1.0.0`: portable ZIPs, NSIS setup EXEs, `SHA256SUMS`, `WIN7.txt`.

## `.github/workflows/ci.yml`

```yaml
name: ci

on:
  push:
    branches: [master]
  pull_request:
  workflow_dispatch:

permissions:
  contents: read

jobs:
  test:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Engine tests
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential
          bash tools/build.sh test
          # build.sh must exit non-zero if any test fails

  lint:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: cppcheck and format
        run: |
          sudo apt-get update
          sudo apt-get install -y cppcheck clang-format
          cppcheck --error-exitcode=2 --enable=warning,style,performance,portability \
            --inline-suppr --suppress=missingIncludeSystem -I src src
          find src tests -name '*.c' -o -name '*.h' | xargs clang-format --dry-run -Werror

  build:
    needs: [test, lint]
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Cross compile
        run: |
          sudo apt-get update
          sudo apt-get install -y gcc-mingw-w64 binutils-mingw-w64 zip nsis
          bash tools/build.sh release
          bash tools/check-imports.sh
      - uses: actions/upload-artifact@v4
        with:
          name: subnetcalc-windows-build
          retention-days: 14
          path: |
            dist/SubnetCalc-Windows-x86.zip
            dist/SubnetCalc-Windows-x64.zip
            dist/SubnetCalc-Windows-x86-setup.exe
            dist/SubnetCalc-Windows-x64-setup.exe
            dist/SHA256SUMS
            dist/imports.txt
            dist/WIN7.txt
```

`tools/check-imports.sh` fails if `x86_64-w64-mingw32-objdump -p` or the i686 objdump prints any of: `msvcp`, `vcruntime`, `ucrtbase`, `libgcc_s`, `libwinpthread`, `bcryptprimitives`. Write the dump to `dist/imports.txt` before the fail check.

`tools/build.sh release` writes the portable ZIPs, both NSIS setup EXEs, and `SHA256SUMS` under `dist/`. NSIS comes from the Ubuntu `nsis` package (`makensis`), not from npm. The script is `installer/SubnetCalc.nsi`, compiled once with `/DARCH=x86` and once with `/DARCH=x64`.

## `.github/workflows/codeql.yml`

```yaml
name: codeql

on:
  push:
    branches: [master]
  pull_request:
  schedule:
    - cron: '17 4 * * 1'
  workflow_dispatch:

permissions:
  contents: read
  security-events: write

jobs:
  analyze:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - uses: github/codeql-action/init@v3
        with:
          languages: c-cpp
          build-mode: manual
      - name: Trace engine build
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential
          bash tools/build.sh test
      - uses: github/codeql-action/analyze@v3
        with:
          category: /language:c-cpp
```

Use `@v4` instead of `@v3` if the default setup page offers v4. Do not set `build-mode: none`.

## `.github/workflows/security.yml`

```yaml
name: security

on:
  push:
    branches: [master]
  pull_request:
  workflow_dispatch:

permissions:
  contents: read

jobs:
  gitleaks:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
        with:
          fetch-depth: 0
      - name: Secret scan
        uses: gitleaks/gitleaks-action@<pin-full-commit-sha>
        env:
          GITHUB_TOKEN: ${{ secrets.GITHUB_TOKEN }}
```

Replace `<pin-full-commit-sha>` with the current `gitleaks-action` commit before the first push. Do not leave the placeholder. Allowlist only the CSV formula-injection test string if gitleaks flags it. Do not allowlist a real token.

This job is the secret scan. CodeQL is the SAST scan. The import check in `ci.yml` is the supply/runtime scan. All three are required.

## `.github/workflows/release.yml`

```yaml
name: release

on:
  push:
    tags: ['v*.*.*']

permissions:
  contents: write

jobs:
  release:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v4
      - name: Test, lint, build
        run: |
          sudo apt-get update
          sudo apt-get install -y build-essential cppcheck clang-format gcc-mingw-w64 binutils-mingw-w64 zip nsis
          bash tools/build.sh test
          cppcheck --error-exitcode=2 --enable=warning,style,performance,portability \
            --inline-suppr --suppress=missingIncludeSystem -I src src
          bash tools/build.sh release
          bash tools/check-imports.sh
      - name: Publish GitHub Release
        env:
          GH_TOKEN: ${{ github.token }}
        run: |
          gh release create "${GITHUB_REF_NAME}" \
            dist/SubnetCalc-Windows-x86.zip \
            dist/SubnetCalc-Windows-x64.zip \
            dist/SubnetCalc-Windows-x86-setup.exe \
            dist/SubnetCalc-Windows-x64-setup.exe \
            dist/SHA256SUMS \
            dist/WIN7.txt \
            --title "SubnetCalc-Windows ${GITHUB_REF_NAME}" \
            --notes-file dist/RELEASE_NOTES.md
```

No `pull_request` trigger. No `if: always()`. A failed import check must not create the release.

## `dist/WIN7.txt` contents

```
SubnetCalc-Windows portable build and NSIS installer.
Portable ZIP runs on Windows 7 SP1 x86 and x64 with no admin and no redistributable.
The setup EXE installs to Program Files and needs administrator. It bundles only the portable EXE, LICENSE, and this file.
No .NET and no Visual C++ Redistributable.
Unsigned. SmartScreen may warn on the portable EXE and on the setup EXE. Check SHA256SUMS before running.
```

## README badges

Point at the workflow files, not at a guessed status URL:

- CI: `https://github.com/alsyundawy/SubnetCalc-Windows/actions/workflows/ci.yml`
- CodeQL: `https://github.com/alsyundawy/SubnetCalc-Windows/security/code-scanning`
- Releases: `https://github.com/alsyundawy/SubnetCalc-Windows/releases`

## Failure rules

- Test failure blocks artifact upload because `build` needs `test` and `lint`.
- CodeQL failure does not block the artifact. It does block the `v1.0.0` tag task until a Critical or High alert is fixed or written down as a false positive with the alert URL.
- Release is never created from a pull request.
