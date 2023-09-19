#ifndef __RISCV__
#define __RISCV__

#include "../common/cpu.h"

struct extension {
  int id;
  char* str;
};

// https://en.wikichip.org/wiki/risc-v/standard_extensions
// Included all except for G
static const struct extension extension_list[] = {
  { 'i' - 'a', "(I) Integer Instruction Set" },
  { 'm' - 'a', "(M) Integer Multiplication and Division" },
  { 'a' - 'a', "(A) Atomic Instructions" },
  { 'f' - 'a', "(F) Single-Precision Floating-Point" },
  { 'd' - 'a', "(D) Double-Precision Floating-Point" },
  { 'q' - 'a', "(Q) Quad-Precision Floating-Point" },
  { 'l' - 'a', "(L) Decimal Floating-Point" },
  { 'c' - 'a', "(C) Compressed Instructions" },
  { 'b' - 'a', "(B) Double-Precision Floating-Point" },
  { 'j' - 'a', "(J) Dynamically Translated Languages" },
  { 't' - 'a', "(T) Transactional Memory" },
  { 'p' - 'a', "(P) Packed-SIMD Instructions" },
  { 'v' - 'a', "(V) Vector Operations" },
  { 'n' - 'a', "(N) User-Level Interrupts" },
  { 'h' - 'a', "(H) Hypervisor" },
  { 's' - 'a', "(S) Supervisor-level Instructions" }
};

#define RISCV_ISA_EXT_MAX	   64
#define RISCV_ISA_EXT_NAME_LEN_MAX 32
#define RISCV_ISA_EXT_BASE         26

// This enum represent the logical ID for multi-letter RISC-V ISA extensions.
// The logical ID should start from RISCV_ISA_EXT_BASE and must not exceed
// RISCV_ISA_EXT_MAX.
enum riscv_isa_ext_id {
  RISCV_ISA_EXT_SSCOFPMF = RISCV_ISA_EXT_BASE,
  RISCV_ISA_EXT_SSTC,
  RISCV_ISA_EXT_SVINVAL,
  RISCV_ISA_EXT_SVPBMT,
  RISCV_ISA_EXT_ZBB,
  RISCV_ISA_EXT_ZICBOM,
  RISCV_ISA_EXT_ZIHINTPAUSE,
  RISCV_ISA_EXT_SVNAPOT,
  RISCV_ISA_EXT_ZICBOZ,
  RISCV_ISA_EXT_SMAIA,
  RISCV_ISA_EXT_SSAIA,
  RISCV_ISA_EXT_ZBA,
  RISCV_ISA_EXT_ZBS,
  RISCV_ISA_EXT_ZICNTR,
  RISCV_ISA_EXT_ZICSR,
  RISCV_ISA_EXT_ZIFENCEI,
  RISCV_ISA_EXT_ZIHPM,
  RISCV_ISA_EXT_ID_MAX
};
static_assert(RISCV_ISA_EXT_ID_MAX <= RISCV_ISA_EXT_MAX);

struct cpuInfo* get_cpu_info(void);
char* get_str_topology(struct cpuInfo* cpu, struct topology* topo);
char* get_str_extensions(struct cpuInfo* cpu);
void print_debug(struct cpuInfo* cpu);

#endif
