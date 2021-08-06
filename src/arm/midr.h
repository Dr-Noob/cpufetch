#ifndef __MIDR__
#define __MIDR__

#include "../common/cpu.h"

struct cpuInfo* get_cpu_info();

uint32_t get_nsockets(struct topology* topo);
char* get_str_topology(struct cpuInfo* cpu, struct topology* topo, bool dual_socket);
char* get_str_features(struct cpuInfo* cpu);

void print_debug(struct cpuInfo* cpu);
void free_topo_struct(struct topology* topo);

// Code taken from cpuinfo (https://github.com/pytorch/cpuinfo/blob/master/src/arm/midr.h)
#define CPUINFO_ARM_MIDR_IMPLEMENTER_MASK  UINT32_C(0xFF000000)
#define CPUINFO_ARM_MIDR_VARIANT_MASK      UINT32_C(0x00F00000)
#define CPUINFO_ARM_MIDR_ARCHITECTURE_MASK UINT32_C(0x000F0000)
#define CPUINFO_ARM_MIDR_PART_MASK         UINT32_C(0x0000FFF0)
#define CPUINFO_ARM_MIDR_REVISION_MASK     UINT32_C(0x0000000F)

#define CPUINFO_ARM_MIDR_IMPLEMENTER_OFFSET  24
#define CPUINFO_ARM_MIDR_VARIANT_OFFSET      20
#define CPUINFO_ARM_MIDR_ARCHITECTURE_OFFSET 16
#define CPUINFO_ARM_MIDR_PART_OFFSET          4
#define CPUINFO_ARM_MIDR_REVISION_OFFSET      0

inline static uint32_t midr_set_implementer(uint32_t midr, uint32_t implementer) {
        return (midr & ~CPUINFO_ARM_MIDR_IMPLEMENTER_MASK) |
                ((implementer << CPUINFO_ARM_MIDR_IMPLEMENTER_OFFSET) & CPUINFO_ARM_MIDR_IMPLEMENTER_MASK);
}

inline static uint32_t midr_set_variant(uint32_t midr, uint32_t variant) {
        return (midr & ~CPUINFO_ARM_MIDR_VARIANT_MASK) |
                ((variant << CPUINFO_ARM_MIDR_VARIANT_OFFSET) & CPUINFO_ARM_MIDR_VARIANT_MASK);
}

inline static uint32_t midr_set_architecture(uint32_t midr, uint32_t architecture) {
        return (midr & ~CPUINFO_ARM_MIDR_ARCHITECTURE_MASK) |
                ((architecture << CPUINFO_ARM_MIDR_ARCHITECTURE_OFFSET) & CPUINFO_ARM_MIDR_ARCHITECTURE_MASK);
}

inline static uint32_t midr_set_part(uint32_t midr, uint32_t part) {
        return (midr & ~CPUINFO_ARM_MIDR_PART_MASK) |
                ((part << CPUINFO_ARM_MIDR_PART_OFFSET) & CPUINFO_ARM_MIDR_PART_MASK);
}

inline static uint32_t midr_set_revision(uint32_t midr, uint32_t revision) {
        return (midr & ~CPUINFO_ARM_MIDR_REVISION_MASK) |
                ((revision << CPUINFO_ARM_MIDR_REVISION_OFFSET) & CPUINFO_ARM_MIDR_REVISION_MASK);
}

inline static uint32_t midr_get_variant(uint32_t midr) {
	return (midr & CPUINFO_ARM_MIDR_VARIANT_MASK) >> CPUINFO_ARM_MIDR_VARIANT_OFFSET;
}

inline static uint32_t midr_get_implementer(uint32_t midr) {
	return (midr & CPUINFO_ARM_MIDR_IMPLEMENTER_MASK) >> CPUINFO_ARM_MIDR_IMPLEMENTER_OFFSET;
}

inline static uint32_t midr_get_part(uint32_t midr) {
	return (midr & CPUINFO_ARM_MIDR_PART_MASK) >> CPUINFO_ARM_MIDR_PART_OFFSET;
}

inline static uint32_t midr_get_revision(uint32_t midr) {
	return (midr & CPUINFO_ARM_MIDR_REVISION_MASK) >> CPUINFO_ARM_MIDR_REVISION_OFFSET;
}

#endif
