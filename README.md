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
Each line in the output means:
node: a list of keys (parent - previous sibling - next sibling)
```bash
$ ./tree 10 0 5 2
-----------------
0x2406c40: 4 (0-0-0)
    0x2406cd0: 0 1 3 4 (0x2406c40-0-0x7fd098000b20)
    0x7fd098000b20: 7 8 (0x2406c40-0x2406cd0-0)
```
