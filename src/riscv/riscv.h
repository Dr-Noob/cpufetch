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

struct cpuInfo* get_cpu_info(void);
char* get_str_topology(struct cpuInfo* cpu, struct topology* topo);
char* get_str_extensions(struct cpuInfo* cpu);
void print_debug(struct cpuInfo* cpu);

#endif
