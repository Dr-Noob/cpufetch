# cpufetch

Prints a fancy summary of the CPU with some advanced information

### Platforms
This tool works on both 64 only and under Linux because of its [implementation details](#implementation). AMD support is not guaranteed so information may not be correct

### Usage and installation

Just clone the repo and use `make` to compile it

```
git clone https://github.com/Dr-Noob/cpufetch
cd cpufetch
make
./cpufetch
```

### Example

This is the output of `cpufetch` in a i7-4790K

![Example](/preview.png)

### Output

Output is detailed as follows:

| Field      | Description             | Possible Values  |
|:----------:|:-----------------------:|:-----------------:|
| Name       | Name of the CPU   | Any valid CPU name |
| Frequency  | Max frequency of the CPU(in GHz) | X.XX(GHz or MHz)
| N.Cores    | Number of cores the CPU has. If CPU supports `Hyperthreading` or similar, this will show cores and threads separately | X(cores)X(threads)
| AVX        | Type of AVX supported by the CPU or None. AVX instructions allows the CPU to vectorize the code with a witdh of 256 bits in single precision(or 512bits if AVX512 is supported) | AVX,AVX2,AVX512,None
| SSE        | Same as AVX, but SSE family are 128bits witdh | SSE, SSE2, SSE3, SSSE3, SSE4a, SSE4_1, SSE4_2,None |
| FMA        | Does this CPU support FMA(Fused Multiply Add)?This instruction allows the CPU to multiply and add a value on the same clock cycle | FMA3,FMA4,None |
| AES        | Does this CPU support AES? This instruction is allows the CPU to make AES cypher efficiently | Yes or No |
| SHA        | Does this CPU support SHA? This instruction is allows the CPU to make SHA hashing efficiently | Yes or No |
| L1 Size    | Size(in bytes) of the L1 cache, separated in data and instructions | XXB(Data)XXB(instructions) |
| L2 Size    | Size(in bytes) of the L2 cache(both are unified) | XXXKB or None |
| L3 Size    | Same as L3 | XXXXKB or None |
| Peak FLOPS | Max FLOPS(Floating Point Operation Per Second) this CPU could theoretical achieve. This is calculated by: `N.Cores*Freq*2(Because 2 functional units)*2(If has FMA)*VectorWidth` | XXX.XX (G/T)FLOPs |

`cpufetch` also prints a simple ascii art of the manufacturer logo.

### Implementation

`cpufetch` makes use of two techniques to fetch data:
* __cpuid__: CPU name, number of threads per core and instructions features are fetched via _cpuid_. See [this](http://www.sandpile.org/x86/cpuid.htm) and [Intel  Processor Identification and the CPUID Instruction](https://www.scss.tcd.ie/~jones/CS4021/processor-identification-cpuid-instruction-note.pdf) for more information.
* __udev__: Cache and frequency are fetched via _udev_, by looking at specific files from `/sys/devices/system/cpu`

### Bugs or improvements
Feel free to open a issue on the repo to report a issue or propose any improvement in the tool
