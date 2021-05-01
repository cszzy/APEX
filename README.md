# APEX: High Performance Learned Index on Persistent Memory

Persistent memory friendly learned index.

## What's included

- APEX - the source code of APEX
- Mini benchmark framework

Fully open-sourced under MIT license.

## Building

### Dependencies
We tested our build with Linux Kernel 5.10.11 and GCC 9.2. You must ensure that your Linux kernel version >= 4.17 and glibc >=2.29 for proper build. 

### Compiling
Assuming to compile under a `build` directory:
```bash
git clone https://github.com/baotonglu/dash.git
cd apex
./build.sh
```

## Running benchmark

We run the tests in a single NUMA node with 24 physical CPU cores. We pin threads to physical cores compactly assuming thread ID == 2 * core ID (e.g., for a dual-socket system, we assume cores 0, 2, 4, ... are located in socket 0).  To run benchmarks, use the `test_pmem` executable in the `build` directory. It supports the following arguments:

Check out also the `total.sh` script for example benchmarks and easy testing of the hash tables. 

## Acknowledgements

Our implementation is based on the code [ALEX](https://github.com/microsoft/ALEX).
