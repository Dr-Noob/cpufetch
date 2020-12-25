### 2. Why differences between Intel and AMD?
There are many different CPUID leaves [[1](#references)]. In some cases, a given leaf does the same thing in Intel and AMD processors, but in the majority of them, they don't. For example, leaf 0x4 gives you the caches information, but in AMD is a reserved (invalid) leaf! In the case of AMD, is more common to fetch information using extended levels than using the standard levels (the other way around with Intel).

### 3. How to get the frequency?
__Involved code: [get_frequency_info (cpuid.c)](https://github.com/Dr-Noob/cpufetch/blob/master/src/x86/cpuid.c)__

CPUID leaf 0x16 is used.

If the CPU does not support supports such level:
- Linux: cpufetch will try to obtain this information using `/sys` filesystem in Linux. I think that Linux knows the frequency using model specific registers (MSRs), which you can't read at the user level.
- Windows: cpufetch can't obtain CPU frequency. This means that peak performance can't be computed because the frequency is needed to compute it.

### 4. How to get cache sizes?
__Involved code: [get_cache_info (cpuid.c)](https://github.com/Dr-Noob/cpufetch/blob/master/src/x86/cpuid.c)__

- Intel: CPUID leaf 0x4 is used (using __get_cache_info_general__). If the CPU does not support it, cpufetch can't get this information.
- AMD: Extended CPUID leaf 0x1D is used (using __get_cache_info_general__). If the CPU does not support this level, cpufetch uses a fallback method, which uses extended leaves 0x5 and 0x6. This fallback method uses __get_cache_info_amd_fallback__.


### 5. How to get CPU microarchitecture?
__Involved code: [get_cpu_uarch (cpuid.c)](https://github.com/Dr-Noob/cpufetch/blob/master/src/x86/cpuid.c), [get_uarch_from_cpuid (uarch.c)](https://github.com/Dr-Noob/cpufetch/blob/master/src/x86/uarch.c)__

CPUID leaf 0x1 is used. From there, we get:
- Model
- Extended Model
- Family
- Extended Family
- Stepping

Knowing this information, we can distinguish any CPU microarchitecture. Inside __uarch.c__ there is a function that behaves like a database or a lookup table. The function of this database is to find a match between the information obtained from 0x1 and what kind of microarchitecture the current CPU is. I got the data using and adapting the code from Todd Allen's cpuid program [[5](#references)]. Knowing the microarchitecture, we can obtain the manufacturing process (or technology, the size in nm of the transistors).

### 6. How to get CPU topology?
__Involved code: [cpuid.h](https://github.com/Dr-Noob/cpufetch/blob/master/src/x86/cpuid.h), [get_topology_info (cpuid.c)](https://github.com/Dr-Noob/cpufetch/blob/master/src/x86/cpuid.c), [apic.c](https://github.com/Dr-Noob/cpufetch/blob/master/src/x86/apic.c)__

cpufetch aims to support the most complex systems, so it supports multi-socket CPUs and detailed SMT (Intel HyperThreading) information. The CPU topology is stored in the following struct:

```
struct topology {
  int64_t total_cores;
  uint32_t physical_cores;
  uint32_t logical_cores;
  uint32_t smt_available;
  uint32_t smt_supported;
  uint32_t sockets;
};
```

This structure needs a bit of explanation, to know what are we looking for:
- `physical_cores`: Number of physical cores. In a multi socket system, this field stores the number of cores for just one socket.
- `logical_cores`: Number of logical cores. In a multi socket system, this field stores the number of logical cores for just one socket.
- `total_cores`: Total number of logical cores. In a multi socket system, this field stores the number of logical cores for the entire system.
- `sockets`: How many sockets the system has.
- `smt_supported`: Stores if SMT (or Intel HT) is supported in the CPU, storing the number of threads per core. So, if `smt_supported == 1`, it means that there is 1 thread per core, and SMT is not supported. If SMT is supported, then `smt_supported >= 1`. Note this field tells if the CPU if supports it, but not if SMT is activated or not.
- `smt_available`: The same idea as `smt_supported`, but it stores if SMT is available. If SMT is not supported, then `smt_available` is always `1`. The differentiation between supported and available lets cpufetch distinguish when a CPU has SMT capabilities, but are disabled (probably in the BIOS).

Let's give two CPU examples and the values that `struct topology` would have in these CPUs.
- Example 1: Dual Socket Intel Xeon 6248:

```
total_cores = 80
physical_cores = 20
logical_cores = 40
smt_available = 2
smt_supported = 2
sockets = 2
```

- Example 2: Intel Core i7-4790K with SMT disabled in BIOS:

```
total_cores = 8
physical_cores = 4
logical_cores = 8
smt_available = 1
smt_supported = 2
sockets = 1
```

Now that we know what data are we looking for, let's see how we get it:

- __Intel__: The methodology used is explained in the Intel webpage [[2](#references)]. Intel explains how to do it and also gives an example source code. I used it and modified it to fit cpufetch style. The core of this methodology is the usage of the APIC id, so the code is inside __apic.c__.
- __AMD__: Intel's algorithm using APIC does not work for AMD. To get the same information in AMD, I used the reference from OSdev [[3](#references)] and also ideas from lscpu [[4](#references)]. This uses:
  - CPUID extended leaf 0x8: Fill `logical_cores`
  - CPUID extended leaf 0x1E: Fill `smt_supported`
  - CPUID standard leaf 0x1 (APIC): Fill `smt_available`

  If any of these levels are not supported, these fields are just guessed. For example, if we are not able to know if SMT is supported, we guess it is not. With all of these data, we can calculate the rest of the fields:

  ```
  physical_cores = logical_cores / smt_available;  
  if(topo->smt_supported > 1)
    sockets = total_cores / smt_supported / physical_cores; // Idea borrowed from lscpu
  else
    sockets = total_cores / physical_cores;    
  ```

### 7. How to get cache topology?
__Involved code: [get_cache_topology_amd (cpuid.c)](https://github.com/Dr-Noob/cpufetch/blob/master/src/x86/cpuid.c), [apic.c](https://github.com/Dr-Noob/cpufetch/blob/master/src/x86/apic.c)__

The topology of a cache gives us information about how many caches we have at a given level. It usually follows the rule of:
- L1: The same as the number of cores (one L1i and one L1d per core).
- L2: If L2 is the last level cache, one L2. If not, the same as the number of cores (one L2 per core).
- L3: One L3 cache per socket (shared among all cores).

These assumptions are generally (but not always) true. For example, for the AMD Zen generation, or the Intel Xeon Phi KNL. Thus, cpufetch does not assume the topology but obtains it instead.

- __Intel__: The idea is similar to the mentioned in CPU topology [[2](#references)](it also covers how to get cache topology using APIC id).

- __AMD__: Again, we have to look for another path for AMD. This time, the way to do it is easier and (I think) more solid and future proof. The idea is to use extended CPUID leaf 0x1D. If the CPU does not support it, we can still guess the topology of the caches (as mentioned earlier). If it does, CPUID can give us how many cores shares a given level of cache. So, if we have the number of cores, we can guess how many caches are there for any given level (see __get_cache_topology_amd__).

#### References
- [1] [sandpile CPUID webpage](https://www.sandpile.org/x86/cpuid.htm)
- [2] [CPU topology and cache topology: Intel](https://software.intel.com/content/www/us/en/develop/articles/intel-64-architecture-processor-topology-enumeration.html)
- [3] [CPU topology: AMD](https://wiki.osdev.org/Detecting_CPU_Topology_(80x86))
- [4] [lscpu](https://github.com/karelzak/util-linux/blob/master/sys-utils/lscpu.c)
- [5] [Todd Allen's cpuid](http://www.etallen.com/cpuid.html)
- [6] [AMD specific CPUID specification](https://www.amd.com/system/files/TechDocs/25481.pdf)
- [7] [Intel vs AMD CPU Architectural Differences: Chips and Chiplets](https://c.mi.com/thread-2585048-1-0.html)

In addition to all these resources, I found it very interesting to search in the Linux kernel source code (for example, the directory [`arch/x86/kernel/cpu/`](https://elixir.bootlin.com/linux/latest/source/arch/x86/kernel/cpu)), because sometimes you can find ideas that cannot be found anywhere else!
