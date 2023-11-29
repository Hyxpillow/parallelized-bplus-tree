# parallelized-bplus-tree

## Introduction

A light-weight parallelized-bplus-tree, implemented by openmp. Support parallelized insert, delete and search.

##  Build dependency

The project use c++17 standard and need to install gcc and libomp to support openmp.
For ubuntu
```bash
apt-get update && apt-get install libomp
```

Build the project
```bash
make
```
## Command line opeartions
The four arguments are number of insertions, deletions, search and number of threads.
Be careful for the opeartions since the b-plus tree malloc memory from the heap.
```bash
./tree num_of_insertions num_of_deletions num_of_search num_of_threads
```
## Output
