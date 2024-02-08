#ifndef __UARCH__
#define __UARCH__

#include <stdint.h>

#include "midr.h"

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
  UARCH_TAISHAN_V110, // HiSilicon TaiShan v110
  UARCH_TAISHAN_V200, // HiSilicon TaiShan v200
  // PHYTIUM
  UARCH_XIAOMI,       // Not to be confused with Xiaomi Inc
};

typedef uint32_t MICROARCH;

struct uarch* get_uarch_from_midr(uint32_t midr, struct cpuInfo* cpu);
int get_number_of_vpus(struct cpuInfo* cpu);
int get_vpus_width(struct cpuInfo* cpu);
bool has_fma_support(struct cpuInfo* cpu);
char* get_str_uarch(struct cpuInfo* cpu);
void free_uarch_struct(struct uarch* arch);
MICROARCH get_uarch(struct uarch* arch);

#endif
