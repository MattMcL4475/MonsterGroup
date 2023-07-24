# In-memory Monster Group

This code allocates a 196,883 x 196,833 matrix of complex numbers on Linux, which requires 577.612 GiB of memory:

```c++
std::vector<std::vector<std::complex<double>>> matrix(196883, std::vector<std::complex<double>>(196883));
```
The matrix represents a linear transformation associated with an element of the Monster Group in its 196,883-dimensional representation. Currently the code simply fills the matrix with random complex numbers.

## Running on Azure

- **SKU**: Standard_E96-24ads_v5 (24 vcpus, 672 GiB memory)
- **OS**: Ubuntu Server 22.04 LTS - Gen2
- **Architecture**: x64
- **Spot price**: $0.6464 / hr

## Compile & run instructions:
*Use the code from: https://github.com/MattMcL4475/MonsterGroup/blob/master/MonsterGroup/main.cpp*
- sudo apt update && sudo apt install g++ -y
- nano monster.cpp
- g++ -o monster monster.cpp
- ./monster

## Result

- Initial memory usage: 0.00196838 GiB / 660.961 GiB (total)
- Memory allocation for 196883x196883 matrix execution time: **9m 30s 5ms**
- Memory usage after operation: **577.854 GiB / 660.961 GiB (total)**

