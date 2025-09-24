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
  RISCV_ISA_EXT_SMSTATEEN,
  RISCV_ISA_EXT_ZICOND,
  RISCV_ISA_EXT_ZBC,
  RISCV_ISA_EXT_ZBKB,
  RISCV_ISA_EXT_ZBKC,
  RISCV_ISA_EXT_ZBKX,
  RISCV_ISA_EXT_ZKND,
  RISCV_ISA_EXT_ZKNE,
  RISCV_ISA_EXT_ZKNH,
  RISCV_ISA_EXT_ZKR,
  RISCV_ISA_EXT_ZKSED,
  RISCV_ISA_EXT_ZKSH,
  RISCV_ISA_EXT_ZKT,
  RISCV_ISA_EXT_ZVBB,
  RISCV_ISA_EXT_ZVBC,
  RISCV_ISA_EXT_ZVKB,
  RISCV_ISA_EXT_ZVKG,
  RISCV_ISA_EXT_ZVKNED,
  RISCV_ISA_EXT_ZVKNHA,
  RISCV_ISA_EXT_ZVKNHB,
  RISCV_ISA_EXT_ZVKSED,
  RISCV_ISA_EXT_ZVKSH,
  RISCV_ISA_EXT_ZVKT,
  RISCV_ISA_EXT_ZFH,
  RISCV_ISA_EXT_ZFHMIN,
  RISCV_ISA_EXT_ZIHINTNTL,
  RISCV_ISA_EXT_ZVFH,
  RISCV_ISA_EXT_ZVFHMIN,
  RISCV_ISA_EXT_ZFA,
  RISCV_ISA_EXT_ZTSO,
  RISCV_ISA_EXT_ZACAS,
  RISCV_ISA_EXT_ZVE32X,
  RISCV_ISA_EXT_ZVE32F,
  RISCV_ISA_EXT_ZVE64X,
  RISCV_ISA_EXT_ZVE64F,
  RISCV_ISA_EXT_ZVE64D,
  RISCV_ISA_EXT_ZIMOP,
  RISCV_ISA_EXT_ZCA,
  RISCV_ISA_EXT_ZCB,
  RISCV_ISA_EXT_ZCD,
  RISCV_ISA_EXT_ZCF,
  RISCV_ISA_EXT_ZCMOP,
  RISCV_ISA_EXT_ZAWRS,
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
  { RISCV_ISA_EXT_SMSTATEEN,   "(Smstateen) State Enable" },
  { RISCV_ISA_EXT_ZICOND,      "(Zicond) Conditional Operations" },
  { RISCV_ISA_EXT_ZBC,         "(Zbc) Basic Carry-less Multiplication" },
  { RISCV_ISA_EXT_ZBKB,        "(Zbkb) Bit-manipulation for Cryptography" },
  { RISCV_ISA_EXT_ZBKC,        "(Zbkc) Carry-less Multiplication for Cryptography" },
  { RISCV_ISA_EXT_ZBKX,        "(Zbkx) Crossbar Permutation for Cryptography" },
  { RISCV_ISA_EXT_ZKND,        "(Zknd) NIST Suite: AES Decryption" },
  { RISCV_ISA_EXT_ZKNE,        "(Zkne) NIST Suite: AES Encryption" },
  { RISCV_ISA_EXT_ZKNH,        "(Zknh) NIST Suite: Hash Functions" },
  { RISCV_ISA_EXT_ZKR,         "(Zkr) Entropy Source Extension" },
  { RISCV_ISA_EXT_ZKSED,       "(Zksed) ShangMi Suite: SM4 Block Cipher" },
  { RISCV_ISA_EXT_ZKSH,        "(Zksh) ShangMi Suite: SM3 Hash Function" },
  { RISCV_ISA_EXT_ZKT,         "(Zkt) Data Independent Timing" },
  { RISCV_ISA_EXT_ZVBB,        "(Zvbb) Vector Basic Bit-manipulation" },
  { RISCV_ISA_EXT_ZVBC,        "(Zvbc) Vector Carry-less Multiplication" },
  { RISCV_ISA_EXT_ZVKB,        "(Zvkb) Vector Bit-manipulation for Cryptography" },
  { RISCV_ISA_EXT_ZVKG,        "(Zvkg) Vector GCM/GMAC" },
  { RISCV_ISA_EXT_ZVKNED,      "(Zvkned) Vector NIST Suite: AES Decryption" },
  { RISCV_ISA_EXT_ZVKNHA,      "(Zvknha) Vector NIST Suite: SHA-256 Hash" },
  { RISCV_ISA_EXT_ZVKNHB,      "(Zvknhb) Vector NIST Suite: SHA-512 Hash" },
  { RISCV_ISA_EXT_ZVKSED,      "(Zvksed) Vector ShangMi Suite: SM4" },
  { RISCV_ISA_EXT_ZVKSH,       "(Zvksh) Vector ShangMi Suite: SM3" },
  { RISCV_ISA_EXT_ZVKT,        "(Zvkt) Vector Data Independent Timing" },
  { RISCV_ISA_EXT_ZFH,         "(Zfh) Half-Precision Floating-Point" },
  { RISCV_ISA_EXT_ZFHMIN,      "(Zfhmin) Half-Precision Floating-Point (Minimal)" },
  { RISCV_ISA_EXT_ZIHINTNTL,   "(Zihintntl) Non-Temporal Locality Hints" },
  { RISCV_ISA_EXT_ZVFH,        "(Zvfh) Vector Half-Precision Floating-Point" },
  { RISCV_ISA_EXT_ZVFHMIN,     "(Zvfhmin) Vector Half-Precision Floating-Point (Minimal)" },
  { RISCV_ISA_EXT_ZFA,         "(Zfa) Additional Floating-Point Instructions" },
  { RISCV_ISA_EXT_ZTSO,        "(Ztso) Total Store Ordering" },
  { RISCV_ISA_EXT_ZACAS,       "(Zacas) Atomic Compare-and-Swap" },
  { RISCV_ISA_EXT_ZVE32X,      "(Zve32x) Vector Extensions for Embedded Systems (32-bit)" },
  { RISCV_ISA_EXT_ZVE32F,      "(Zve32f) Vector Extensions for Embedded Systems (32-bit + FP)" },
  { RISCV_ISA_EXT_ZVE64X,      "(Zve64x) Vector Extensions for Embedded Systems (64-bit)" },
  { RISCV_ISA_EXT_ZVE64F,      "(Zve64f) Vector Extensions for Embedded Systems (64-bit + FP)" },
  { RISCV_ISA_EXT_ZVE64D,      "(Zve64d) Vector Extensions for Embedded Systems (64-bit + DP)" },
  { RISCV_ISA_EXT_ZIMOP,       "(Zimop) Integer Move Operations" },
  { RISCV_ISA_EXT_ZCA,         "(Zca) Compressed Instructions (A)" },
  { RISCV_ISA_EXT_ZCB,         "(Zcb) Compressed Instructions (B)" },
  { RISCV_ISA_EXT_ZCD,         "(Zcd) Compressed Instructions (D)" },
  { RISCV_ISA_EXT_ZCF,         "(Zcf) Compressed Instructions (F)" },
  { RISCV_ISA_EXT_ZCMOP,       "(Zcmop) Compressed Move Operations" },
  { RISCV_ISA_EXT_ZAWRS,       "(Zawrs) Atomic WRS (Wait-Reservation-Set)" }
};

struct cpuInfo* get_cpu_info(void);
char* get_str_topology(struct cpuInfo* cpu, struct topology* topo);
char* get_str_extensions(struct cpuInfo* cpu);
void print_debug(struct cpuInfo* cpu);

#endif
