### 2. Why differences between Intel and AMD?
There are many different CPUID leaves (see [Useful documentation](#useful-documentation)). However, there are cases where a given leaf does the same thing in Intel and AMD, but the majority of them, they don't. For example, leaf 0x4 gives you the caches information, but in
AMD is a reserved (invalid) leaf! In the case of AMD, is more common to fetch information using extended levels than using the standard levels (the other way around with Intel).

### 3. How to get the frequency?
__Involved code: get_frequency_info (cpuid.c)__

We use CPUID leaf 0x16.

If the CPU supports it, we are done. If not:
- Linux: cpufetch will try to obtain this information using `/sys` filesystem in Linux. I think that, in this case, Linux knows the frequency using MSRs, so at the user level you can't know it, but I will look at the kernel source code to check it.
- Windows: We are dead. cpufetch can't fetch CPU frequency. This means that peak performance can't be computed because the frequency is needed to compute it.

### 4. How to get cache sizes?
__Involved code: get_cache_info (cpuid.c)__

- Intel: We use CPUID leaf 0x4
- AMD: We use extended CPUID leaf 0x1D

If CPU does not support it, we are dead: cpufetch can't get this information. If CPU does, we can fetch it, and 0x4 in Intel behaves the same way as 0x1D in AMD.

### 5. How to get CPU microarchitecture?
__Involved code: get_cpu_uarch (cpuid.c), get_uarch_from_cpuid (uarch.c)__

We use CPUID leaf 0x1. From there, we get:
- Model
- Extended Model
- Family
- Extended Family
- Stepping

Knowing this information, we can distinguish any CPU microarchitecture. Inside __uarch.c__ there is a function that behaves as it had a database. It will search, for this information, what kind of microarchitecture the current CPU is. I got the data using and adapting the code form Todd Allen's cpuid program (Link 5 in [Useful documentation](#useful-documentation)). From the microarchitecture, we can obtain the manufacturing process (or technology, the size in nm of the transistors).

### 6. How to get CPU topology?
__Involved code: cpuid.h, get_topology_info (cpuid.c), apic.c__

cpufetch tries to support the most complex systems, so it supports multi socket CPUs and a detailed SMT (Intel HyperThreading) information. The CPU topology is stored like this:

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
- `smt_supported`: Stores if SMT (or Intel HT) is supported in the CPU, storing the number of threads per core. So, if `smt_supported == 1`, it means that there is 1 thread per core, and SMT is not supported. If SMT is supported, then `smt_supported >= 1`. Note this field tells if CPU if supports it, but not if SMT is activated or not.
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

- __Intel__: We use the methodology explained in the Intel webpage (Link 2 in [Useful documentation](#useful-documentation)). Here, Intel explains how to do it and also gives an example source code. I used it and modified it to fit cpufetch style. The core of this methodology is the usage of the APIC id, so the code is inside __apic.c__.
- __AMD__: Intel's algorithm using APIC does not work for AMD. To get the same information in AMD, I used the reference from OSdev(Link 3 in [Useful documentation](#useful-documentation)) and also ideas from lscpu(Link 4 in [Useful documentation](#useful-documentation)). This uses:
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
__Involved code: get_cache_topology_amd (cpuid.c), apic.c__

The topology is reduced to the idea of how many caches we have at a given level. It usually follows the rule of:
- L1: The same as the number of cores (one L1i and one L1d per core).
- L2: If L2 is the last level cache, one L2. If not, the same as the number of cores (one L2 per core).
- L3: One L3 cache per socket (shared among all cores).

These assumptions are generally (but not always) true. For example, for the AMD Zen generation, or the Intel Xeon Phi KNL. Thus, cpufetch does not assume the topology and tries to fetch it instead.

- __Intel__: We can use the methodology explained in the Intel webpage (Link 2 in [Useful documentation](#useful-documentation)) again (it also covers how to get cache topology using APIC id).

- __AMD__: Again, we have to look another path for AMD. This time, the way to do it is easier and (I think) more solid and future proof. We just use extended CPUID leaf 0x1D. If the CPU does not support it, we can still guess the topology of the caches (as mentioned earlier). If it does, CPUID can give us how many cores shares a given level of cache. So, if we have the number of cores, we can guess how many caches are there for any given level (see __get_cache_topology_amd__).

#### Useful documentation
- Link 1: [sandpile CPUID webpage](https://www.sandpile.org/x86/cpuid.htm)
- Link 2: [CPU topology and cache topology: Intel](https://software.intel.com/content/www/us/en/develop/articles/intel-64-architecture-processor-topology-enumeration.html)
- Link 3: [CPU topology: AMD](https://wiki.osdev.org/Detecting_CPU_Topology_(80x86))
- Link 4: [lscpu](https://github.com/karelzak/util-linux/blob/master/sys-utils/lscpu.c)
- Link 5: [Todd Allen's cpuid](http://www.etallen.com/cpuid.html)
- Link 6: [AMD specific CPUID specification](https://www.amd.com/system/files/TechDocs/25481.pdf)

In addition to all these resources, I found very interesting to search in the Linux kernel source code (for example, the directory `arch/x86/kernel/cpu/`), because sometetimes you can find ideas that cannot be found anywhere else!
