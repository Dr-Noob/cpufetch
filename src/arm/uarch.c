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

struct uarch {
  MICROARCH uarch;
  char* uarch_str;
  // int32_t process; process depends on SoC
};

enum cpuinfo_uarch {
  UARCH_UNKNOWN,
  // ARM
  UARCH_ARM7,
  UARCH_ARM9,  
  UARCH_ARM11,       // ARM 1136, ARM 1156, ARM 1176, or ARM 11MPCore.
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
  UARCH_CORTEX_A53r0, // ARM Cortex-A55 revision 0 (restricted dual-issue capabilities compared to revision 1+).
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


#define UARCH_START if (false) {}
#define CHECK_UARCH(arch, cpu, im_, p_, v_, cpuname, str, uarch, vendor) \
   else if (im_ == im && p_ == p && (v_ == NA || v_ == v)) fill_uarch(arch, cpu, cpuname, str, uarch, vendor);
#define UARCH_END else { printBug("Unknown microarchitecture detected: IM=0x%.8X P=0x%.8X V=0x%.8X", im, p, v); fill_uarch(arch, cpu, "Unknown", "Unknown", UARCH_UNKNOWN, CPU_VENDOR_UNKNOWN); }
   
void fill_uarch(struct uarch* arch, struct cpuInfo* cpu, char* cpuname, char* str, MICROARCH u, VENDOR vendor) {
  arch->uarch_str = malloc(sizeof(char) * (strlen(str)+1));
  strcpy(arch->uarch_str, str);
  arch->uarch = u;
  cpu->cpu_vendor = vendor;
  cpu->cpu_name = malloc(sizeof(char) * (strlen(cpuname)+1));
  strcpy(cpu->cpu_name, cpuname);
}   

struct uarch* get_uarch_from_midr(uint32_t midr, struct cpuInfo* cpu) {
  struct uarch* arch = malloc(sizeof(struct uarch));
  uint32_t im = midr_get_implementer(midr);
  uint32_t p = midr_get_part(midr);
  uint32_t v = midr_get_variant(midr);
  
  // IM: Implementer                                                         //
  // P:  Part                                                                //
  // V:  Variant                                                             //
  // ----------------------------------------------------------------------- //
  //                      IM   P       V                                     //
  UARCH_START  
  CHECK_UARCH(arch, cpu, 'A',  0xD03,  NA, "Cortex-A53",     "ARM v7l", UARCH_CORTEX_A53, CPU_VENDOR_ARM)
  CHECK_UARCH(arch, cpu, 'B',  0xD03,  NA, "Cortex-Unknown", "ARM v7l", UARCH_CORTEX_A35, CPU_VENDOR_ARM)
  UARCH_END
  
  return arch;
}

char* get_str_uarch(struct cpuInfo* cpu) {
  return cpu->arch->uarch_str;    
}

char* get_str_process(struct cpuInfo* cpu) {
  char* str = malloc(sizeof(char) * (4+2+1));
  sprintf(str, "%s", "Unknown");
  return str;
}

void free_uarch_struct(struct uarch* arch) {    
  free(arch->uarch_str);
  free(arch);
}

