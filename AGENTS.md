# AGENTS.md

Guidance for coding agents working in this repository.

## Project Overview
- `cpufetch` is a C command-line tool that prints CPU/vendor/microarchitecture information.
- Build selection is host-driven: the `Makefile` compiles one architecture target based on host arch/OS.
- Architecture support in source is split across `x86`, `arm`, `ppc`, and `riscv`, with shared logic in `src/common`.

## Repository Layout
- `src/common/`: CLI parsing, shared structs, formatting, common OS helpers.
- `src/x86/`: CPUID/APIC/topology/cache/frequency detection for x86/x86_64.
- `src/arm/`: MIDR + Linux sysfs/proc parsing, SoC detection, ARM-specific logic.
- `src/ppc/`: PVR + Linux-based PowerPC detection.
- `src/riscv/`: RISC-V vendor/uarch/SoC detection.
- `doc/`: implementation notes (`DOCUMENTATION_X86.md`, `DOCUMENTATION_ARM.md`, etc.).
- `cpufetch.1`: man page.

## Build, Run, and Validation
- `make`: release-style build (`-O2`).
- `make debug`: debug build (`-g -O0`).
- `make strict`: stricter warnings + UBSan (`-Werror -fsanitize=undefined`).
- `make clean`: remove binary/objects.
- `./cpufetch`: run tool.
- `./cpufetch --help`, `./cpufetch --version`, `./cpufetch --debug`: quick smoke checks.

For code changes, prefer this minimum validation sequence:
1. `make clean && make`
2. `make strict` (when toolchain/platform supports it)
3. Run `./cpufetch --version` and at least one functional run (`./cpufetch` or `./cpufetch --debug`)

## Editing Conventions
- Keep C standard/style consistent with existing files (mostly C99/gnu99 depending on target).
- Preserve existing preprocessor structure (`#ifdef ARCH_X86`, `ARCH_ARM`, `ARCH_PPC`, `ARCH_RISCV`, OS guards).
- Use project allocation/error helpers from `src/common/global.c` (`emalloc`, `ecalloc`, `erealloc`, `printErr`, `printBug`) instead of ad-hoc patterns.
- Reuse existing constants/sentinels (for example `UNKNOWN_DATA`) and existing structs in `src/common/cpu.h`.
- Keep changes architecture-scoped where possible: avoid touching unrelated backends.

## Architecture-Specific Notes
- x86 detection primarily uses CPUID and APIC (`src/x86/cpuid.c`, `src/x86/apic.c`, `src/x86/uarch.c`).
- ARM detection relies on Linux `/proc` + `/sys` and MIDR reconstruction (`src/arm/udev.c`, `src/arm/midr.c`, `src/arm/soc.c`).
- PPC detection uses PVR and Linux helpers (`src/ppc/ppc.c`, `src/ppc/uarch.c`).
- RISC-V logic is under `src/riscv/` with SoC/uarch matching similar to ARM patterns.

## Helper Scripts
- `src/x86/uarch_decode.sh`: helper for decoding CPUID tuple fields.
- `src/arm/socs_generation.sh`: helper for generating SoC enum-style entries.
- `src/ppc/pvr_kern_to_cpufetch.sh`: helper for transforming Linux kernel PVR tables.

Treat these scripts as developer utilities; verify generated output before committing.

## Upstream Workflow Context
- Upstream repository is configured to lock/close pull requests and expects contribution via issues (`CONTRIBUTING.md`).
- If preparing a patch for upstream discussion, include reproducible commands and `cpufetch --debug` output in notes.
