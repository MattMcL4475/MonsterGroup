# In-memory Monster Group

This code allocates a 196,883 x 196,833 matrix on Linux, which requires 577.612 GiB of memory. The matrix represents a linear transformation associated with an element of the Monster Group in its 196,883-dimensional representation. Currently the code simply fills the matrix with random complex numbers.

## Running on Azure

- **SKU**: Standard E96 - 24ads v5 (24 vcpus, 672 GiB memory)
- **OS**: Ubuntu Server 22.04 LTS - Gen2
- **Architecture**: x64
- **Spot price**: $0.6464 / hr

## Compile & run instructions:

- sudo apt update && sudo apt install g++ -y
- nano monster.cpp
- g++ -o monster monster.cpp
- ./monster

## Result

- Initial memory usage: 0.00196838 GiB / 660.961 GiB (total)
- Memory allocation for 196883x196883 matrix execution time: **9m 30s 5ms**
- Memory usage after operation: **577.854 GiB / 660.961 GiB (total)**

