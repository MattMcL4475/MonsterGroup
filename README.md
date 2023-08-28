# In-memory monster group
This code defines a memory-aligned 196,883 x 196,833 matrix of complex numbers on Linux (577.6 GiB):

```c++
real = static_cast<double*>(aligned_alloc(64, 196883 * 196883 * sizeof(double)));
imag = static_cast<double*>(aligned_alloc(64, 196883 * 196883 * sizeof(double)));
```

The matrix represents a linear transformation associated with an element of the monster group in its 196,883-dimensional representation. This code currently generates two of these random matrices and multiplies them using an array of CPU optimizations.

## Hardware requirements
- **OS**: Ubuntu Server 22.04 LTS - Gen2, Windows 10/11/Server
- **Architecture**: x64 *(w/ optional AVX-512)*
- **Memory**: >1,734 GiB
- **Cores**: max available

## Compile and run
### Linux
```bash
sudo apt update && sudo apt install g++ make -y
git clone https://github.com/MattMcL4475/MonsterGroup.git
cd MonsterGroup/MonsterGroup
make
./monster
```
### Windows
```cmd
git clone https://github.com/MattMcL4475/MonsterGroup.git
cd MonsterGroup
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" MonsterGroup.sln /p:Configuration=Release
.\Release\monster.exe
```

## Matrix multiplication optimizations
- **Memory Alignment**<br/>
64-byte aligned memory allocations for real and imaginary arrays optimized for [Intel® AVX-512](https://www.intel.com/content/www/us/en/architecture-and-technology/avx-512-overview.html).
- **SIMD Vectorization (with AVX-512)**<br/>
Processes eight doubles at a time using AVX-512 instructions in the inner loop, including aligned data load/store and fused multiply-add (FMA) operations
- **Multithreading**<br/>
Uses ```std::thread``` to maximize concurrency on all available hyperthreads.
- **Loop Unrolling**<br/>
In the AVX-512 code section, the k-loop is unrolled in groups of four to maximize parallelism.
- **Prefetching**<br/>
Includes pre-fetch hints so the CPU pre-fetches data into the cache before it's accessed.
- **Cache Blocking / Tiling (in non-AVX-512 code)**<br/>
The non-AVX-512 code organizes loops into blocks or "tiles" to fit the working set of data into the CPU cache. The three nested loops are divided into blocks of size ```tileSize```, which is dynamically calculated based on the cache line size of the hardware.
- **Conditional platform compilation**<br/>
Platform-specific implementations for both Linux and Windows.

## Reference
"It's got too many properties for it to be an accident.  It's never been explained why it's even there" -[John Horton Conway FRS](https://en.wikipedia.org/wiki/John_Horton_Conway) (26 December 1937 – 11 April 2020)

**Monster group**: _https://en.wikipedia.org/wiki/Monster_group_

**Monsterous moonshine**: https://en.wikipedia.org/wiki/Monstrous_moonshine

