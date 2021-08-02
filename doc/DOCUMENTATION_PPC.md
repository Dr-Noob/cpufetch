### 2. How to get CPU microarchitecture?
__Involved code: [get_uarch_from_pvr (uarch.c)](https://github.com/Dr-Noob/cpufetch/src/ppc/uarch.c)__

Microarchitecture is deduced from the PVR register, which is read using the `mfpvr` instruction. The correspondence between the PVR and the specific microarchitecture has been implemented using the values in `arch/powerpc/kernel/cputable.c` in the Linux kernel. Some of them have been removed. The manufacturing process has been queried by searching on the internet.

### 3. How to get CPU topology?
__Involved code: [get_topology_info (ppc.c)](https://github.com/Dr-Noob/cpufetch/src/ppc/ppc.c)__

The total number of cores is queried using `sysconf(_SC_NPROCESSORS_ONLN)`. Then, with the number of sockets and the number of physical cores, we can calculate the number of threads per core.

The number of sockets is queried using `/sys/devices/system/cpu/cpu*/topology/physical_package_id`. Once this file has been read for all of the cores, a simple custom algorithm is used to determine the number of sockets.

The number of physical cores is queried using `/sys/devices/system/cpu/cpu*/topology/core_id`. Again, a custom algorithm is used to determine the number of physical cores.

### 4. How to get the frequency?
Frequency is read directly from `/sys/devices/system/cpu/cpu*/cpufreq/cpuinfo_max_freq`

### 5. How to get cache size and topology?
Cache size is retrieved directly from Linux (using `/sys/devices/system/cpu/cpu0/cache/index*/size`).

To find the cache topology, the files `/sys/devices/system/cpu/cpu0/cache/index*/shared_cpu_map` are used, and a custom algorithm is used to determine how many caches are there at each level.

_NOTE_: To avoid Linux dependencies at this point, it looks like it is possible to derive the cache size and topology from the microarchitecture. For example, in the POWER9 architecture, wikichip assumes that all the POWER9 CPUs have the same cache size for each core and topology [[1](#references)].

#### References
- [1] [POWER9 - wikichip](https://en.wikichip.org/wiki/ibm/microarchitectures/power9)
