#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "uarch.h"
#include "../common/global.h"

#define STRING_UNKNOWN    "Unknown"

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
  UARCH_NEOVERSE_N1,
  UARCH_NEOVERSE_E1,
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
  // CAVIUM
  UARCH_THUNDERX,   // Cavium ThunderX
  UARCH_THUNDERX2,  //  Cavium ThunderX2 (originally Broadcom Vulkan).
  // MARVELL
  UARCH_PJ4,
  UARCH_BRAHMA_B15,
  UARCH_BRAHMA_B53,
  UARCH_XGENE,        // Applied Micro X-Gene.
  UARCH_TAISHAN_V110  // HiSilicon TaiShan v110 (Huawei Kunpeng 920 series processors).
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
  [UARCH_NEOVERSE_N1]  = ISA_ARMv8_2_A,
  [UARCH_NEOVERSE_E1]  = ISA_ARMv8_2_A,
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
  [UARCH_ICESTORM]     = ISA_ARMv8_4_A,
  [UARCH_FIRESTORM]    = ISA_ARMv8_4_A,
  [UARCH_PJ4]          = ISA_ARMv7_A,
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
  [ISA_ARMv8_4_A] = "ARMv8.4"
};

#define UARCH_START if (false) {}
#define CHECK_UARCH(arch, cpu, im_, p_, v_, r_, str, uarch, vendor) \
   else if (im_ == im && p_ == p && (v_ == NA || v_ == v) && (r_ == NA || r_ == r)) fill_uarch(arch, cpu, str, uarch, vendor);
#define UARCH_END else { printBug("Unknown microarchitecture detected: IM=0x%.8X P=0x%.8X V=0x%.8X R=0x%.8X", im, p, v, r); fill_uarch(arch, cpu, "Unknown", UARCH_UNKNOWN, CPU_VENDOR_UNKNOWN); }
   
void fill_uarch(struct uarch* arch, struct cpuInfo* cpu, char* str, MICROARCH u, VENDOR vendor) {
  arch->uarch = u;  
  arch->isa = isas_uarch[arch->uarch];
  cpu->cpu_vendor = vendor;
  
  arch->uarch_str = malloc(sizeof(char) * (strlen(str)+1));
  strcpy(arch->uarch_str, str);
  
  arch->isa_str = malloc(sizeof(char) * (strlen(isas_string[arch->isa])+1));
  strcpy(arch->isa_str, isas_string[arch->isa]);  
}   

/*
 * Codes are based on pytorch/cpuinfo, more precisely:
 * - https://github.com/pytorch/cpuinfo/blob/master/src/arm/uarch.c
 * Other sources:
 * - https://elixir.bootlin.com/linux/latest/source/arch/arm64/include/asm/cputype.h
 * - https://elixir.bootlin.com/linux/latest/source/arch/arm/include/asm/cputype.h
 */
struct uarch* get_uarch_from_midr(uint32_t midr, struct cpuInfo* cpu) {
  struct uarch* arch = malloc(sizeof(struct uarch));
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
  CHECK_UARCH(arch, cpu, 'A', 0xD41, NA, NA, "Cortex-A78",            UARCH_CORTEX_A78,   CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'A', 0xD4A, NA, NA, "Neoverse E1",           UARCH_NEOVERSE_E1,  CPU_VENDOR_ARM)
     
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

  CHECK_UARCH(arch, cpu, 'a', 0x022, NA, NA, "Icestorm",              UARCH_ICESTORM,     CPU_VENDOR_APPLE)
  CHECK_UARCH(arch, cpu, 'a', 0x023, NA, NA, "Firestorm",             UARCH_FIRESTORM,    CPU_VENDOR_APPLE)

  CHECK_UARCH(arch, cpu, 'V', 0x581, NA, NA, "PJ4",                   UARCH_PJ4,          CPU_VENDOR_MARVELL)
  CHECK_UARCH(arch, cpu, 'V', 0x584, NA, NA, "PJ4B-MP",               UARCH_PJ4,          CPU_VENDOR_MARVELL)
  
  UARCH_END
  
  return arch;
}

char* get_str_uarch(struct cpuInfo* cpu) {
  return cpu->arch->uarch_str;    
}

void free_uarch_struct(struct uarch* arch) {    
  free(arch->uarch_str);
  free(arch);
}

