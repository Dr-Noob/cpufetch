#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define MSR_PSTATE_0 0xC0010064

// Reads a 64-bit MSR value from a given CPU core
int read_msr(int cpu, uint64_t msr_addr, uint64_t *value) {
    char msr_path[64];
    snprintf(msr_path, sizeof(msr_path), "/dev/cpu/%d/msr", cpu);

    int fd = open(msr_path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    if (lseek(fd, msr_addr, SEEK_SET) == -1) {
        perror("lseek");
        close(fd);
        return -1;
    }

    if (read(fd, value, sizeof(*value)) != sizeof(*value)) {
        perror("read");
        close(fd);
        return -1;
    }

    close(fd);
    return 0;
}

int main(int argc, char **argv) {
    int cpu = 0;
    if (argc > 1) cpu = atoi(argv[1]);

    uint64_t val;
    if (read_msr(cpu, MSR_PSTATE_0, &val) != 0) {
        fprintf(stderr, "Failed to read MSR on CPU %d\n", cpu);
        return 1;
    }

    unsigned int fid = val & 0xff;          // bits [7:0]
    unsigned int did = (val >> 8) & 0x3f;   // bits [13:8]
    double freq_mhz = (fid * 25.0) / (1 << did);  // 25 MHz ref clock on Zen CPUs

    printf("CPU%d: MSR[0x%lx] = 0x%016llx\n", cpu, (unsigned long)MSR_PSTATE_0, (unsigned long long)val);
    printf("FID: %u, DID: %u\n", fid, did);
    printf("Estimated frequency: %.2f MHz\n", freq_mhz);

    return 0;
}
