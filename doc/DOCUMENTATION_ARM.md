### 2. Why ARM cpufetch works on Linux based systems?
CPUID instructions (present in x86 architectures) [[1](#references)] allow user level applications to obtain information about the CPU. In ARM architectures, there are many registers (MIDR [[2](#references)], CCSIDR [[3](#references)]) that provide information about the CPU too. However, those registers can only be read from privilege mode (PL1 or higher). Therefore, any user level tool which can actually read information about the running CPU must use the operating system to do so. cpufetch uses some Linux kernel features (see the remaining sections). Therefore, cpufetch in ARM processors is limited to Linux kernel based systems, such as __GNU/Linux__ and __Android__

### 3. How to get CPU microarchitecture?
__Involved code: [get_midr_from_cpuinfo (udev.c)](https://github.com/Dr-Noob/cpufetch/blob/master/src/arm/udev.c), [midr.c](https://github.com/Dr-Noob/cpufetch/blob/master/src/arm/midr.c)__

Microarchitecture information is acquired from the Main ID Register (MIDR) [[2](#references)]. Currently, cpufetch rebuilds this register using `/proc/cpuinfo` file. While this file does not contain the value of the register per se, it contains the following fields:
- `CPU implementer`,
- `CPU architecture`
- `CPU variant`
- `CPU part`
- `CPU revision`

The MIDR register can be built with this information. Another posible approach is to read MIDR directly from `/sys/devices/system/cpu/cpu*/regs/identification/midr_el1`

With the MIDR available, the approach is the same as the one used in x86_64 architectures. cpufetch has a file that acts like a database that tries to match the MIDR register with the specific CPU microarchitecture.

### 4. How to get CPU topology?
__Involved code: [get_ncores_from_cpuinfo (udev.c)](https://github.com/Dr-Noob/cpufetch/blob/master/src/arm/udev.c), [midr.c](https://github.com/Dr-Noob/cpufetch/blob/master/src/arm/midr.c)__

ARM provides a new interesting architecture feature: big.LITTLE architectures [[4](#references)]. An ARM CPU can be organized like a typical x86_64 CPU, where all cores share the same microarchitecture. However, ARM big.LITTLE architecture breaks this schema. In a big.LITTLE CPU, two or more CPUs microarchitecture live in the same chip.

This means that cpufetch can't just read which microarchitecture is the first core and assume that the rest of them shares the same microarchitecture. To get the CPU topology, cpufetch first reads the number of CPU cores. This can be obtained from `/sys/devices/system/cpu/present`

Then, for each core, cpufetch reads the MIDR and also the frequency (see section 5). Then, cpufetch assumes that two cores are different when their MIDR are different. This idea allows cpufetch to detect big.LITTLE architectures, and to know how many cores of each architecture the running CPU has.

### 5. How to get the frequency?
Frequency is read directly from `/sys/devices/system/cpu/cpu*/cpufreq/cpuinfo_max_freq`

### 6. How to get system on chip model?
__Involved code: [soc.c](https://github.com/Dr-Noob/cpufetch/blob/master/src/arm/soc.c)__

System on chip (SoC) model is obtained using the same idea as the microarchitecture. First, SoC string is read. Then, the string has to be matched against a database-like function (__parse_soc_from_string__). The SoC string of the running CPU can be obtained using two different approaches:

- Using `/proc/cpuinfo`. This is the first thing to try. Linux kernel usually provides the string under the `Hardware` keyword. However, the Linux kernel may be unable to provide this information, or this string may not be found in the database-like function.
- Using Android properties: This only works on Android systems. Android properties can be read using `__system_property_get` function. cpufetch tries to read two properties:
  - `ro.mediatek.platform`
  - `ro.product.board`

  If any string is returned, cpufetch tries to find a match in the database (using the same database as in the case of `/proc/cpuinfo`).

The expected strings have to be hardcoded. I found two ways of knowing which string should correspond to which SoC:

  - Searching on the internet. Manufacturers __usually__ provide this information. For example, Qualcomm usually publishes the chip name along with other characteristics (under the `Part` or `Part number` keyword [[6](#references)]).
  - "Hunting" for the strings. For example, finding smartphones with a given SoC and manually reading the `/proc/cpuinfo` or the `build.prop` file. A very good resource to do this is the SpecDevice webpage [[7](#references)]).

### 7. How to get cache size and topology?
ARM architecture supports reading the cache information via some registers (for example, the CCSIDR register  [[3](#references)]). As mentioned earlier, user level applications are not able to read these registers directly. The remaining option is to ask the operating system for this information. However, at the moment, the __Linux kernel does not provide cache information__. Therefore, cpufetch does not print any cache information on ARM CPUs at the moment. There are, however, other approaches to be explored:

- Read the registers in kernel mode. This can be accomplished by running a kernel module [[4](#references)]. Unfortunately, running a custom kernel module is tricky, and sometimes impossible to do reasonably (for example, in Android devices). In any case, my decision is to run cpufetch on user level only.
- Hardcode the cache information for each SoC: Sometimes, manufacturers publish technical information about the chips, where cache topology and size are shown. This method is impractical, since this kind of information is very hard (or impossible) to find online, and the number of SoC is huge.

#### References
- [1] [cpufetch x86_64 documentation](https://github.com/Dr-Noob/cpufetch/blob/master/doc/DOCUMENTATION_X86.md)
- [2] [Main ID Register](https://developer.arm.com/documentation/ddi0433/c/system-control/register-descriptions/main-id-register)
- [3] [Cache size ID Register](https://developer.arm.com/documentation/100403/0200/register-descriptions/aarch32-system-registers/ccsidr--cache-size-id-register)
- [4] [How to get the size of the CPU cache in Linux](https://stackoverflow.com/a/63474811/9975463)
- [5] [ARM big.LITTLE](https://en.wikipedia.org/wiki/ARM_big.LITTLE)
- [6] [Snapdragon 855+ Mobile Platform](https://www.qualcomm.com/products/snapdragon-855-plus-mobile-platform)
- [7] [SpecDevice](http://specdevice.com/unmoderated.php)
