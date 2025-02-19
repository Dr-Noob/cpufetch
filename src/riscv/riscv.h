#ifndef __RISCV__
#define __RISCV__

#include "../common/cpu.h"

struct extension {
  int id;
  char* str;
};

#define RISCV_ISA_EXT_NAME_LEN_MAX 32
#define RISCV_ISA_EXT_BASE         26

// https://elixir.bootlin.com/linux/latest/source/arch/riscv/include/asm/hwcap.h
// This enum represent the logical ID for multi-letter RISC-V ISA extensions.
// The logical ID should start from RISCV_ISA_EXT_BASE
enum riscv_isa_ext_id {
  RISCV_ISA_EXT_SSCOFPMF = RISCV_ISA_EXT_BASE,
  RISCV_ISA_EXT_SSTC,
  RISCV_ISA_EXT_SVINVAL,
  RISCV_ISA_EXT_SVPBMT,
  RISCV_ISA_EXT_ZBB,
  RISCV_ISA_EXT_ZICBOM,
  RISCV_ISA_EXT_ZIHINTPAUSE,
  RISCV_ISA_EXT_SVNAPOT,
  RISCV_ISA_EXT_ZICBOP,
  RISCV_ISA_EXT_ZICBOZ,
  RISCV_ISA_EXT_SMAIA,
  RISCV_ISA_EXT_SSAIA,
  RISCV_ISA_EXT_ZBA,
  RISCV_ISA_EXT_ZBS,
  RISCV_ISA_EXT_ZICNTR,
  RISCV_ISA_EXT_ZICSR,
  RISCV_ISA_EXT_ZIFENCEI,
  RISCV_ISA_EXT_ZIHPM,
  RISCV_ISA_EXT_ZCA,
  RISCV_ISA_EXT_ZCD,
  RISCV_ISA_EXT_ID_MAX
};

// https://five-embeddev.com/riscv-isa-manual/latest/preface.html#preface
// https://en.wikichip.org/wiki/risc-v/standard_extensions
// (Zicbop) https://github.com/riscv/riscv-CMOs/blob/master/cmobase/Zicbop.adoc
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
  // multi-letter extensions
  { RISCV_ISA_EXT_SSCOFPMF,    "(Sscofpmf) Count OverFlow and Privilege Mode Filtering" },
  { RISCV_ISA_EXT_SSTC,        "(Sstc) S and VS level Time Compare" },
  { RISCV_ISA_EXT_SVINVAL,     "(Svinval) Fast TLB Invalidation" },
  { RISCV_ISA_EXT_SVPBMT,      "(Svpbmt) Page-based Memory Types" },
  { RISCV_ISA_EXT_ZBB,         "(Zbb) Basic bit-manipulation" },
  { RISCV_ISA_EXT_ZICBOM,      "(Zicbom) Cache Block Management Operations" },
  { RISCV_ISA_EXT_ZIHINTPAUSE, "(Zihintpause) Pause Hint" },
  { RISCV_ISA_EXT_SVNAPOT,     "(Svnapot) Naturally Aligned Power of Two Pages" },
  { RISCV_ISA_EXT_ZICBOZ,      "(Zicboz) Cache Block Zero Operations" },
  { RISCV_ISA_EXT_ZICBOP,      "(Zicbop) Cache Block Prefetch Operations" },
  { RISCV_ISA_EXT_SMAIA,       "(Smaia) Advanced Interrupt Architecture" },
  { RISCV_ISA_EXT_SSAIA,       "(Ssaia) Advanced Interrupt Architecture" },
  { RISCV_ISA_EXT_ZBA,         "(Zba) Address Generation" },
  { RISCV_ISA_EXT_ZBS,         "(Zbs) Single-bit Instructions" },
  { RISCV_ISA_EXT_ZICNTR,      "(Zicntr) Base Counters and Timers" },
  { RISCV_ISA_EXT_ZICSR,       "(Zicsr) Control and Status Register" },
  { RISCV_ISA_EXT_ZIFENCEI,    "(Zifencei) Instruction-Fetch Fence" },
  { RISCV_ISA_EXT_ZIHPM,       "(Zihpm) Hardware Performance Counters" },
  { RISCV_ISA_EXT_ZCA,         "(Zca) Integer Compressed Instructions" },
  { RISCV_ISA_EXT_ZCD,         "(Zcd) Double FP Compressed Instructions" }
};

struct cpuInfo* get_cpu_info(void);
char* get_str_topology(struct cpuInfo* cpu, struct topology* topo);
char* get_str_extensions(struct cpuInfo* cpu);
void print_debug(struct cpuInfo* cpu);

#endif
