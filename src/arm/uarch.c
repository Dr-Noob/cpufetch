#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "uarch.h"
#include "../common/global.h"

// Data not available
#define NA                   -1

typedef uint32_t MICROARCH;
typedef uint32_t ISA;

struct uarch {
  MICROARCH uarch;
  ISA isa;
  char* uarch_str;
  char* isa_str;
  // int32_t process; process depends on SoC
};

enum {
  ISA_ARMv6,
  ISA_ARMv6_T2,
  ISA_ARMv6_KZ,
  ISA_ARMv6_K,
  ISA_ARMv7_A,
  ISA_ARMv8_A,
  ISA_ARMv8_A_AArch32,
  ISA_ARMv8_1_A,
  ISA_ARMv8_2_A,
  ISA_ARMv8_3_A,
  ISA_ARMv8_4_A,
  ISA_ARMv8_5_A,
  ISA_ARMv9_A
};

enum {
  UARCH_UNKNOWN,
  // ARM
  UARCH_ARM7,
  UARCH_ARM9,
  UARCH_ARM1136,
  UARCH_ARM1156,
  UARCH_ARM1176,
  UARCH_ARM11MPCORE,
  UARCH_CORTEX_A5,
  UARCH_CORTEX_A7,
  UARCH_CORTEX_A8,
  UARCH_CORTEX_A9,
  UARCH_CORTEX_A12,
  UARCH_CORTEX_A15,
  UARCH_CORTEX_A17,
  UARCH_CORTEX_A32,
  UARCH_CORTEX_A35,
  UARCH_CORTEX_A53,
  UARCH_CORTEX_A55r0, // ARM Cortex-A55 revision 0 (restricted dual-issue capabilities compared to revision 1+).
  UARCH_CORTEX_A55,
  UARCH_CORTEX_A57,
  UARCH_CORTEX_A65,
  UARCH_CORTEX_A72,
  UARCH_CORTEX_A73,
  UARCH_CORTEX_A75,
  UARCH_CORTEX_A76,
  UARCH_CORTEX_A77,
  UARCH_CORTEX_A78,
  UARCH_CORTEX_A510,
  UARCH_CORTEX_A710,
  UARCH_CORTEX_A715,
  UARCH_CORTEX_X1,
  UARCH_CORTEX_X2,
  UARCH_CORTEX_X3,
  UARCH_NEOVERSE_N1,
  UARCH_NEOVERSE_E1,
  UARCH_NEOVERSE_V1,
  UARCH_SCORPION,
  UARCH_KRAIT,
  UARCH_KYRO,
  UARCH_FALKOR,
  UARCH_SAPHIRA,
  UARCH_DENVER,
  UARCH_DENVER2,
  UARCH_CARMEL,
  // SAMSUNG
  UARCH_EXYNOS_M1,  // Samsung Exynos M1 (Exynos 8890 big cores)
  UARCH_EXYNOS_M2,  // Samsung Exynos M2 (Exynos 8895 big cores)
  UARCH_EXYNOS_M3,  // Samsung Exynos M3 (Exynos 9810 big cores)
  UARCH_EXYNOS_M4,  // Samsung Exynos M4 (Exynos 9820 big cores)
  UARCH_EXYNOS_M5,  // Samsung Exynos M5 (Exynos 9830 big cores)
  // APPLE
  UARCH_SWIFT,      // Apple A6 and A6X processors.
  UARCH_CYCLONE,    // Apple A7 processor.
  UARCH_TYPHOON,    // Apple A8 and A8X processor
  UARCH_TWISTER,    // Apple A9 and A9X processor.
  UARCH_HURRICANE,  // Apple A10 and A10X processor.
  UARCH_MONSOON,    // Apple A11 processor (big cores).
  UARCH_MISTRAL,    // Apple A11 processor (little cores).
  UARCH_VORTEX,     // Apple A12 processor (big cores).
  UARCH_TEMPEST,    // Apple A12 processor (big cores).
  UARCH_LIGHTNING,  // Apple A13 processor (big cores).
  UARCH_THUNDER,    // Apple A13 processor (little cores).
  UARCH_ICESTORM,   // Apple M1 processor (little cores).
  UARCH_FIRESTORM,  // Apple M1 processor (big cores).
  UARCH_BLIZZARD,   // Apple M2 processor (little cores).
  UARCH_AVALANCHE,  // Apple M2 processor (big cores).
  UARCH_SAWTOOTH,   // Apple M3 processor (little cores).
  UARCH_EVEREST,    // Apple M3 processor (big cores).
  // CAVIUM
  UARCH_THUNDERX,   // Cavium ThunderX
  UARCH_THUNDERX2,  // Cavium ThunderX2 (originally Broadcom Vulkan).
  // MARVELL
  UARCH_PJ4,
  UARCH_BRAHMA_B15,
  UARCH_BRAHMA_B53,
  UARCH_XGENE,        // Applied Micro X-Gene.
  UARCH_TAISHAN_V110, // HiSilicon TaiShan v110 (Huawei Kunpeng 920 series processors).
  // PHYTIUM
  UARCH_XIAOMI,       // Not to be confused with Xiaomi Inc
};

static const ISA isas_uarch[] = {
  [UARCH_ARM1136]      = ISA_ARMv6,
  [UARCH_ARM1156]      = ISA_ARMv6_T2,
  [UARCH_ARM1176]      = ISA_ARMv6_KZ,
  [UARCH_ARM11MPCORE]  = ISA_ARMv6_K,
  [UARCH_CORTEX_A5]    = ISA_ARMv7_A,
  [UARCH_CORTEX_A7]    = ISA_ARMv7_A,
  [UARCH_CORTEX_A8]    = ISA_ARMv7_A,
  [UARCH_CORTEX_A9]    = ISA_ARMv7_A,
  [UARCH_CORTEX_A12]   = ISA_ARMv7_A,
  [UARCH_CORTEX_A15]   = ISA_ARMv7_A,
  [UARCH_CORTEX_A17]   = ISA_ARMv7_A,
  [UARCH_CORTEX_A32]   = ISA_ARMv8_A_AArch32,
  [UARCH_CORTEX_A35]   = ISA_ARMv8_A,
  [UARCH_CORTEX_A53]   = ISA_ARMv8_A,
  [UARCH_CORTEX_A55r0] = ISA_ARMv8_2_A,
  [UARCH_CORTEX_A55]   = ISA_ARMv8_2_A,
  [UARCH_CORTEX_A57]   = ISA_ARMv8_A,
  [UARCH_CORTEX_A65]   = ISA_ARMv8_2_A,
  [UARCH_CORTEX_A72]   = ISA_ARMv8_A,
  [UARCH_CORTEX_A73]   = ISA_ARMv8_A,
  [UARCH_CORTEX_A75]   = ISA_ARMv8_2_A,
  [UARCH_CORTEX_A76]   = ISA_ARMv8_2_A,
  [UARCH_CORTEX_A77]   = ISA_ARMv8_2_A,
  [UARCH_CORTEX_A78]   = ISA_ARMv8_2_A,
  [UARCH_CORTEX_A510]   = ISA_ARMv9_A,
  [UARCH_CORTEX_A710]   = ISA_ARMv9_A,
  [UARCH_CORTEX_A715]  = ISA_ARMv9_A,
  [UARCH_CORTEX_X1]    = ISA_ARMv8_2_A,
  [UARCH_CORTEX_X2]    = ISA_ARMv9_A,
  [UARCH_CORTEX_X3]    = ISA_ARMv9_A,
  [UARCH_NEOVERSE_N1]  = ISA_ARMv8_2_A,
  [UARCH_NEOVERSE_E1]  = ISA_ARMv8_2_A,
  [UARCH_NEOVERSE_V1]  = ISA_ARMv8_4_A,
  [UARCH_BRAHMA_B15]   = ISA_ARMv7_A,   // Same as Cortex-A15
  [UARCH_BRAHMA_B53]   = ISA_ARMv8_A,   // Same as Cortex-A53
  [UARCH_THUNDERX]     = ISA_ARMv8_A,
  [UARCH_THUNDERX2]    = ISA_ARMv8_1_A,
  [UARCH_TAISHAN_V110] = ISA_ARMv8_2_A,
  [UARCH_DENVER]       = ISA_ARMv8_A,
  [UARCH_DENVER2]      = ISA_ARMv8_A,
  [UARCH_CARMEL]       = ISA_ARMv8_A,
  [UARCH_XGENE]        = ISA_ARMv8_A, // https://en.wikichip.org/wiki/apm/x-gene
  [UARCH_SCORPION]     = ISA_ARMv7_A, // https://www.geektopia.es/es/product/qualcomm/snapdragon-s3-apq8060/
  [UARCH_KRAIT]        = ISA_ARMv7_A,
  [UARCH_KYRO]         = ISA_ARMv8_A,
  [UARCH_FALKOR]       = ISA_ARMv8_A,
  [UARCH_SAPHIRA]      = ISA_ARMv8_3_A,
  [UARCH_EXYNOS_M1]    = ISA_ARMv8_A,
  [UARCH_EXYNOS_M2]    = ISA_ARMv8_A,
  [UARCH_EXYNOS_M3]    = ISA_ARMv8_A,
  [UARCH_EXYNOS_M4]    = ISA_ARMv8_2_A,
  [UARCH_EXYNOS_M5]    = ISA_ARMv8_2_A,
  [UARCH_ICESTORM]     = ISA_ARMv8_5_A, // https://github.com/llvm/llvm-project/blob/main/llvm/include/llvm/Support/AArch64TargetParser.def
  [UARCH_FIRESTORM]    = ISA_ARMv8_5_A,
  [UARCH_BLIZZARD]     = ISA_ARMv8_5_A, // Not confirmed
  [UARCH_AVALANCHE]    = ISA_ARMv8_5_A,
  [UARCH_PJ4]          = ISA_ARMv7_A,
  [UARCH_XIAOMI]       = ISA_ARMv8_A,
};

static char* isas_string[] = {
  [ISA_ARMv6]  = "ARMv6",
  [ISA_ARMv6_T2] = "ARMv6T2",
  [ISA_ARMv6_KZ] = "ARMv6KZ",
  [ISA_ARMv6_K] = "ARMv6K",
  [ISA_ARMv7_A] = "ARMv7",
  [ISA_ARMv8_A] = "ARMv8",
  [ISA_ARMv8_A_AArch32] = "ARMv8 AArch32",
  [ISA_ARMv8_1_A] = "ARMv8.1",
  [ISA_ARMv8_2_A] = "ARMv8.2",
  [ISA_ARMv8_3_A] = "ARMv8.3",
  [ISA_ARMv8_4_A] = "ARMv8.4",
  [ISA_ARMv8_5_A] = "ARMv8.5",
  [ISA_ARMv9_A] = "ARMv9"
};

#define UARCH_START if (false) {}
#define CHECK_UARCH(arch, cpu, im_, p_, v_, r_, str, uarch, vendor) \
   else if (im_ == im && p_ == p && (v_ == NA || v_ == v) && (r_ == NA || r_ == r)) fill_uarch(arch, cpu, str, uarch, vendor);
#define UARCH_END else { printBugCheckRelease("Unknown microarchitecture detected: IM=0x%X P=0x%X V=0x%X R=0x%X", im, p, v, r); \
fill_uarch(arch, cpu, "Unknown", UARCH_UNKNOWN, CPU_VENDOR_UNKNOWN); }

void fill_uarch(struct uarch* arch, struct cpuInfo* cpu, char* str, MICROARCH u, VENDOR vendor) {
  arch->uarch = u;
  arch->isa = isas_uarch[arch->uarch];
  cpu->cpu_vendor = vendor;

  arch->uarch_str = emalloc(sizeof(char) * (strlen(str)+1));
  strcpy(arch->uarch_str, str);

  arch->isa_str = emalloc(sizeof(char) * (strlen(isas_string[arch->isa])+1));
  strcpy(arch->isa_str, isas_string[arch->isa]);
}

/*
 * Codes are based on pytorch/cpuinfo, more precisely:
 * - https://github.com/pytorch/cpuinfo/blob/main/src/arm/uarch.c
 * Other sources:
 * - https://elixir.bootlin.com/linux/latest/source/arch/arm64/include/asm/cputype.h
 * - https://elixir.bootlin.com/linux/latest/source/arch/arm/include/asm/cputype.h
 * - https://github.com/AsahiLinux/m1n1/blob/main/src/chickens.c
 */
struct uarch* get_uarch_from_midr(uint32_t midr, struct cpuInfo* cpu) {
  struct uarch* arch = emalloc(sizeof(struct uarch));
  uint32_t im = midr_get_implementer(midr);
  uint32_t p = midr_get_part(midr);
  uint32_t v = midr_get_variant(midr);
  uint32_t r = midr_get_revision(midr);

  // ----------------------------------------------------------------------- //
  // IM: Implementer                                                         //
  // P:  Part                                                                //
  // V:  Variant                                                             //
  // R:  Revision                                                            //
  // ----------------------------------------------------------------------- //
  //                     IM   P      V   R                                   //
  UARCH_START
  CHECK_UARCH(arch, cpu, 'A', 0xB36, NA, NA, "ARM1136",               UARCH_ARM1136,      CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xB56, NA, NA, "ARM1156",               UARCH_ARM1156,      CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xB76, NA, NA, "ARM1176",               UARCH_ARM1176,      CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xB02, NA, NA, "ARM11 MPCore",          UARCH_ARM11MPCORE,  CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xC05, NA, NA, "Cortex-A5",             UARCH_CORTEX_A5,    CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xC07, NA, NA, "Cortex-A7",             UARCH_CORTEX_A7,    CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xC08, NA, NA, "Cortex-A8",             UARCH_CORTEX_A8,    CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xC09, NA, NA, "Cortex-A9",             UARCH_CORTEX_A9,    CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xC0C, NA, NA, "Cortex-A12",            UARCH_CORTEX_A12,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xC0E, NA, NA, "Cortex-A17",            UARCH_CORTEX_A17,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xC0D, NA, NA, "Cortex-A12",            UARCH_CORTEX_A12,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xC0F, NA, NA, "Cortex-A15",            UARCH_CORTEX_A15,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD01, NA, NA, "Cortex-A32",            UARCH_CORTEX_A32,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD03, NA, NA, "Cortex-A53",            UARCH_CORTEX_A53,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD04, NA, NA, "Cortex-A35",            UARCH_CORTEX_A35,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD05, NA,  0, "Cortex-A55",            UARCH_CORTEX_A55r0, CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD05, NA, NA, "Cortex-A55",            UARCH_CORTEX_A55,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD06, NA, NA, "Cortex-A65",            UARCH_CORTEX_A65,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD07, NA, NA, "Cortex-A57",            UARCH_CORTEX_A57,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD08, NA, NA, "Cortex-A72",            UARCH_CORTEX_A72,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD09, NA, NA, "Cortex-A73",            UARCH_CORTEX_A73,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD0A, NA, NA, "Cortex-A75",            UARCH_CORTEX_A75,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD0B, NA, NA, "Cortex-A76",            UARCH_CORTEX_A76,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD0C, NA, NA, "Neoverse N1",           UARCH_NEOVERSE_N1,  CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD0D, NA, NA, "Cortex-A77",            UARCH_CORTEX_A77,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD0E, NA, NA, "Cortex-A76",            UARCH_CORTEX_A76,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD40, NA, NA, "Neoverse V1",           UARCH_NEOVERSE_V1,  CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD41, NA, NA, "Cortex-A78",            UARCH_CORTEX_A78,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD44, NA, NA, "Cortex-X1",             UARCH_CORTEX_X1,    CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD46, NA, NA, "Cortex‑A510",           UARCH_CORTEX_A510,  CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD47, NA, NA, "Cortex‑A710",           UARCH_CORTEX_A710,  CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD48, NA, NA, "Cortex-X2",             UARCH_CORTEX_X2,    CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD4A, NA, NA, "Neoverse E1",           UARCH_NEOVERSE_E1,  CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD4D, NA, NA, "Cortex-A715",           UARCH_CORTEX_A715,  CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD4E, NA, NA, "Cortex-X3",             UARCH_CORTEX_X3,    CPU_VENDOR_ARM)

  CHECK_UARCH(arch, cpu, 'B', 0x00F, NA, NA, "Brahma B15",            UARCH_BRAHMA_B15,   CPU_VENDOR_BROADCOM)
  CHECK_UARCH(arch, cpu, 'B', 0x100, NA, NA, "Brahma B53",            UARCH_BRAHMA_B53,   CPU_VENDOR_BROADCOM)
  CHECK_UARCH(arch, cpu, 'B', 0x516, NA, NA, "ThunderX2",             UARCH_THUNDERX2,    CPU_VENDOR_CAVIUM)

  CHECK_UARCH(arch, cpu, 'C', 0x0A0, NA, NA, "ThunderX",              UARCH_THUNDERX,     CPU_VENDOR_CAVIUM)
  CHECK_UARCH(arch, cpu, 'C', 0x0A1, NA, NA, "ThunderX 88XX",         UARCH_THUNDERX,     CPU_VENDOR_CAVIUM)
  CHECK_UARCH(arch, cpu, 'C', 0x0A2, NA, NA, "ThunderX 81XX",         UARCH_THUNDERX,     CPU_VENDOR_CAVIUM)
  CHECK_UARCH(arch, cpu, 'C', 0x0A3, NA, NA, "ThunderX 81XX",         UARCH_THUNDERX,     CPU_VENDOR_CAVIUM)
  CHECK_UARCH(arch, cpu, 'C', 0x0AF, NA, NA, "ThunderX2 99XX",        UARCH_THUNDERX2,    CPU_VENDOR_CAVIUM)

  CHECK_UARCH(arch, cpu, 'H', 0xD01, NA, NA, "TaiShan v110",          UARCH_TAISHAN_V110, CPU_VENDOR_HUAWUEI) // Kunpeng 920 series
  CHECK_UARCH(arch, cpu, 'H', 0xD40, NA, NA, "Cortex-A76",            UARCH_CORTEX_A76,   CPU_VENDOR_ARM)     // Kirin 980 Big/Medium cores -> Cortex-A76

  CHECK_UARCH(arch, cpu, 'N', 0x000, NA, NA, "Denver",                UARCH_DENVER,       CPU_VENDOR_NVIDIA)
  CHECK_UARCH(arch, cpu, 'N', 0x003, NA, NA, "Denver2",               UARCH_DENVER2,      CPU_VENDOR_NVIDIA)
  CHECK_UARCH(arch, cpu, 'N', 0x004, NA, NA, "Carmel",                UARCH_CARMEL,       CPU_VENDOR_NVIDIA)

  CHECK_UARCH(arch, cpu, 'P', 0x000, NA, NA, "Xgene",                 UARCH_XGENE,        CPU_VENDOR_APM)

  CHECK_UARCH(arch, cpu, 'Q', 0x00F, NA, NA, "Scorpion",              UARCH_SCORPION,     CPU_VENDOR_QUALCOMM)
  CHECK_UARCH(arch, cpu, 'Q', 0x02D, NA, NA, "Scorpion",              UARCH_KRAIT,        CPU_VENDOR_QUALCOMM)
  CHECK_UARCH(arch, cpu, 'Q', 0x04D,  1,  0, "Krait 200",             UARCH_KRAIT,        CPU_VENDOR_QUALCOMM)
  CHECK_UARCH(arch, cpu, 'Q', 0x04D,  1,  4, "Krait 200",             UARCH_KRAIT,        CPU_VENDOR_QUALCOMM)
  CHECK_UARCH(arch, cpu, 'Q', 0x04D,  2,  0, "Krait 300",             UARCH_KRAIT,        CPU_VENDOR_QUALCOMM)
  CHECK_UARCH(arch, cpu, 'Q', 0x06F,  0,  1, "Krait 200",             UARCH_KRAIT,        CPU_VENDOR_QUALCOMM)
  CHECK_UARCH(arch, cpu, 'Q', 0x06F,  0,  2, "Krait 200",             UARCH_KRAIT,        CPU_VENDOR_QUALCOMM)
  CHECK_UARCH(arch, cpu, 'Q', 0x06F,  1,  0, "Krait 300",             UARCH_KRAIT,        CPU_VENDOR_QUALCOMM)
  CHECK_UARCH(arch, cpu, 'Q', 0x06F,  2,  0, "Krait 400",             UARCH_KRAIT,        CPU_VENDOR_QUALCOMM) // Snapdragon 800 MSMxxxx
  CHECK_UARCH(arch, cpu, 'Q', 0x06F,  2,  1, "Krait 400",             UARCH_KRAIT,        CPU_VENDOR_QUALCOMM) // Snapdragon 801 MSMxxxxPRO
  CHECK_UARCH(arch, cpu, 'Q', 0x06F,  3,  1, "Krait 450",             UARCH_KRAIT,        CPU_VENDOR_QUALCOMM)
  CHECK_UARCH(arch, cpu, 'Q', 0x201, NA, NA, "Kryo Silver",           UARCH_KYRO,         CPU_VENDOR_QUALCOMM) // Qualcomm Snapdragon 821: Low-power Kryo "Silver"
  CHECK_UARCH(arch, cpu, 'Q', 0x205, NA, NA, "Kryo Gold",             UARCH_KYRO,         CPU_VENDOR_QUALCOMM) // Qualcomm Snapdragon 820 & 821: High-performance Kryo "Gold"
  CHECK_UARCH(arch, cpu, 'Q', 0x211, NA, NA, "Kryo Silver",           UARCH_KYRO,         CPU_VENDOR_QUALCOMM) // Qualcomm Snapdragon 820: Low-power Kryo "Silver"
  CHECK_UARCH(arch, cpu, 'Q', 0x800, 10, NA, "Kryo 260 / 280 Gold",   UARCH_CORTEX_A73,   CPU_VENDOR_ARM)      // Kryo 260 / Kryo 280 "Gold"
  CHECK_UARCH(arch, cpu, 'Q', 0x801, 10, NA, "Kryo 260 / 280 Silver", UARCH_CORTEX_A53,   CPU_VENDOR_ARM)      // Kryo 260 / 280 "Silver"
  CHECK_UARCH(arch, cpu, 'Q', 0x802, NA, NA, "Kryo 385 Gold",         UARCH_CORTEX_A75,   CPU_VENDOR_ARM)      // High-performance Kryo 385 "Gold" -> Cortex-A75
  CHECK_UARCH(arch, cpu, 'Q', 0x803, NA, NA, "Kryo 385 Silver",       UARCH_CORTEX_A55r0, CPU_VENDOR_ARM)      // Low-power Kryo 385 "Silver" -> Cortex-A55r0
  CHECK_UARCH(arch, cpu, 'Q', 0x804, NA, NA, "Kryo 485 Gold",         UARCH_CORTEX_A76,   CPU_VENDOR_ARM)      // High-performance Kryo 485 "Gold" / "Gold Prime" -> Cortex-A76
  CHECK_UARCH(arch, cpu, 'Q', 0x805, NA, NA, "Kryo 485 Silver",       UARCH_CORTEX_A55,   CPU_VENDOR_ARM)      // Low-performance Kryo 485 "Silver" -> Cortex-A55
  CHECK_UARCH(arch, cpu, 'Q', 0xC00, NA, NA, "Falkor",                UARCH_FALKOR,       CPU_VENDOR_QUALCOMM)
  CHECK_UARCH(arch, cpu, 'Q', 0xC01, NA, NA, "Saphira",               UARCH_SAPHIRA,      CPU_VENDOR_QUALCOMM)

  CHECK_UARCH(arch, cpu, 'S', 0x001, 1,  NA, "Exynos M1",             UARCH_EXYNOS_M1,    CPU_VENDOR_SAMSUNG)   // Exynos 8890
  CHECK_UARCH(arch, cpu, 'S', 0x001, 4,  NA, "Exynos M2",             UARCH_EXYNOS_M2,    CPU_VENDOR_SAMSUNG)   // Exynos 8895
  CHECK_UARCH(arch, cpu, 'S', 0x002, 1,  NA, "Exynos M3",             UARCH_EXYNOS_M3,    CPU_VENDOR_SAMSUNG)   // Exynos 9810
  CHECK_UARCH(arch, cpu, 'S', 0x003, 1,  NA, "Exynos M4",             UARCH_EXYNOS_M4,    CPU_VENDOR_SAMSUNG)   // Exynos 9820
  CHECK_UARCH(arch, cpu, 'S', 0x004, 1,  NA, "Exynos M5",             UARCH_EXYNOS_M5,    CPU_VENDOR_SAMSUNG)   // Exynos 9820 (this one looks wrong at uarch.c ...)

  CHECK_UARCH(arch, cpu, 'p', 0x663, 1,  NA, "Xiaomi",                UARCH_XIAOMI,       CPU_VENDOR_PHYTIUM)   // From a fellow contributor (https://github.com/Dr-Noob/cpufetch/issues/125)
                                                                                                                // Also interesting: https://en.wikipedia.org/wiki/FeiTeng_(processor)

  CHECK_UARCH(arch, cpu, 'a', 0x022, NA, NA, "Icestorm",              UARCH_ICESTORM,     CPU_VENDOR_APPLE)
  CHECK_UARCH(arch, cpu, 'a', 0x023, NA, NA, "Firestorm",             UARCH_FIRESTORM,    CPU_VENDOR_APPLE)
  CHECK_UARCH(arch, cpu, 'a', 0x030, NA, NA, "Blizzard",              UARCH_BLIZZARD,     CPU_VENDOR_APPLE)
  CHECK_UARCH(arch, cpu, 'a', 0x031, NA, NA, "Avalanche",             UARCH_AVALANCHE,    CPU_VENDOR_APPLE)
  CHECK_UARCH(arch, cpu, 'a', 0x048, NA, NA, "Sawtooth",              UARCH_SAWTOOTH,     CPU_VENDOR_APPLE)
  CHECK_UARCH(arch, cpu, 'a', 0x049, NA, NA, "Everest",               UARCH_EVEREST,      CPU_VENDOR_APPLE)

  CHECK_UARCH(arch, cpu, 'V', 0x581, NA, NA, "PJ4",                   UARCH_PJ4,          CPU_VENDOR_MARVELL)
  CHECK_UARCH(arch, cpu, 'V', 0x584, NA, NA, "PJ4B-MP",               UARCH_PJ4,          CPU_VENDOR_MARVELL)

  UARCH_END

  return arch;
}

bool is_ARMv8_or_newer(struct cpuInfo* cpu) {
  return cpu->arch->isa == ISA_ARMv8_A         ||
         cpu->arch->isa == ISA_ARMv8_A_AArch32 ||
         cpu->arch->isa == ISA_ARMv8_1_A       ||
         cpu->arch->isa == ISA_ARMv8_2_A       ||
         cpu->arch->isa == ISA_ARMv8_3_A       ||
         cpu->arch->isa == ISA_ARMv8_4_A       ||
         cpu->arch->isa == ISA_ARMv8_5_A       ||
         cpu->arch->isa == ISA_ARMv9_A;
}

bool has_fma_support(struct cpuInfo* cpu) {
  // Arm A64 Instruction Set Architecture
  // https://developer.arm.com/documentation/ddi0596/2021-12/SIMD-FP-Instructions
  return is_ARMv8_or_newer(cpu);
}

int get_vpus_width(struct cpuInfo* cpu) {
  // If the CPU has NEON, width can be 64 or 128 [1].
  // In >= ARMv8, NEON are 128 bits width [2]
  // If the CPU has SVE/SVE2, width can be between 128-2048 [3],
  // so we must check the exact width depending on
  // the exact chip (Neoverse V1 uses 256b implementations.)
  //
  // [1] https://en.wikipedia.org/wiki/ARM_architecture_family#Advanced_SIMD_(Neon)
  // [2] https://developer.arm.com/documentation/102474/0100/Fundamentals-of-Armv8-Neon-technology
  // [3] https://www.anandtech.com/show/16640/arm-announces-neoverse-v1-n2-platforms-cpus-cmn700-mesh/5

  MICROARCH ua = cpu->arch->uarch;
  switch(ua) {
    case UARCH_NEOVERSE_V1:
      return 256;
    default:
      if(cpu->feat->NEON) {
        if(is_ARMv8_or_newer(cpu)) {
          return 128;
        }
        else {
          return 64;
        }
      }
      else {
        return 32;
      }
  }
}

int get_number_of_vpus(struct cpuInfo* cpu) {
  MICROARCH ua = cpu->arch->uarch;

  switch(ua) {
    case UARCH_EVEREST:     // Just a guess, needs confirmation.
    case UARCH_FIRESTORM:   // [https://dougallj.github.io/applecpu/firestorm-simd.html]
    case UARCH_AVALANCHE:   // [https://en.wikipedia.org/wiki/Comparison_of_ARM_processors]
    case UARCH_CORTEX_X1:   // [https://www.anandtech.com/show/15813/arm-cortex-a78-cortex-x1-cpu-ip-diverging/3]
    case UARCH_CORTEX_X2:   // [https://www.anandtech.com/show/16693/arm-announces-mobile-armv9-cpu-microarchitectures-cortexx2-cortexa710-cortexa510/2]
    case UARCH_CORTEX_X3:   // [https://www.hwcooling.net/en/cortex-x3-the-new-fastest-arm-core-architecture-analysis: "The FPU and SIMD unit of the core still has four pipelines"]
    case UARCH_NEOVERSE_V1: // [https://en.wikichip.org/wiki/arm_holdings/microarchitectures/neoverse_v1]
      return 4;
    case UARCH_SAWTOOTH:    // Needs confirmation, rn this is the best we know: https://mastodon.social/@dougall/111118317031041336
    case UARCH_EXYNOS_M3:   // [https://www.anandtech.com/show/12361/samsung-exynos-m3-architecture]
    case UARCH_EXYNOS_M4:   // [https://en.wikichip.org/wiki/samsung/microarchitectures/m4#Block_Diagram]
    case UARCH_EXYNOS_M5:   // [https://en.wikichip.org/wiki/samsung/microarchitectures/m5]
      return 3;
    case UARCH_ICESTORM:    // [https://dougallj.github.io/applecpu/icestorm-simd.html]
    case UARCH_BLIZZARD:    // [https://en.wikipedia.org/wiki/Comparison_of_ARM_processors]
    case UARCH_CORTEX_A57:  // [https://www.anandtech.com/show/8718/the-samsung-galaxy-note-4-exynos-review/5]
    case UARCH_CORTEX_A72:  // [https://www.anandtech.com/show/10347/arm-cortex-a73-artemis-unveiled/2]
    case UARCH_CORTEX_A73:  // [https://www.anandtech.com/show/10347/arm-cortex-a73-artemis-unveiled/2]
    case UARCH_CORTEX_A75:  // [https://www.anandtech.com/show/11441/dynamiq-and-arms-new-cpus-cortex-a75-a55/3]
    case UARCH_CORTEX_A76:  // [https://www.anandtech.com/show/12785/arm-cortex-a76-cpu-unveiled-7nm-powerhouse/3]
    case UARCH_CORTEX_A77:  // [https://fuse.wikichip.org/news/2339/arm-unveils-cortex-a77-emphasizes-single-thread-performance]
    case UARCH_CORTEX_A78:  // [https://fuse.wikichip.org/news/3536/arm-unveils-the-cortex-a78-when-less-is-more]
    case UARCH_EXYNOS_M1:   // [https://www.anandtech.com/show/12361/samsung-exynos-m3-architecture]
    case UARCH_EXYNOS_M2:   // [https://www.anandtech.com/show/12361/samsung-exynos-m3-architecture]
    case UARCH_NEOVERSE_N1: // [https://en.wikichip.org/wiki/arm_holdings/microarchitectures/neoverse_n1#Individual_Core]
    case UARCH_CORTEX_A710: // [https://chipsandcheese.com/2023/08/11/arms-cortex-a710-winning-by-default/]: Fig in Core Overview. Table in Instruction Scheduling and Execution
    case UARCH_CORTEX_A715: // [https://www.hwcooling.net/en/arm-introduces-new-cortex-a715-core-architecture-analysis/]: "the numbers of ALU and FPU execution units themselves >
      return 2;
    case UARCH_NEOVERSE_E1: // [https://www.anandtech.com/show/13959/arm-announces-neoverse-n1-platform/5]
    // A510 is integrated as part of a Complex. Normally, each complex would incorporate two Cortex-A510 cores.
    // Each complex incorporates a single VPU with 2 ports, so for each A510 there is theoretically 1 port.
    case UARCH_CORTEX_A510: // [https://en.wikichip.org/wiki/arm_holdings/microarchitectures/cortex-a510#Vector_Processing_Unit_.28VPU.29]
      return 1;
    default:
      // ARMv6
      // ARMv7
      // Remaining UARCH_CORTEX_AXX
      // Old Snapdragon (e.g., Scorpion, Krait, etc)
      return 1;
  }
}

char* get_str_uarch(struct cpuInfo* cpu) {
  return cpu->arch->uarch_str;
}

void free_uarch_struct(struct uarch* arch) {
  free(arch->uarch_str);
  free(arch);
}
