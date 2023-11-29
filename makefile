CC := g++
CFLAGS := -std=c++17 -fopenmp -g
source_files := bptree.cpp
include_files := parallel_rwlatch.hpp bptree.h
zip_name := bptree.zip

tree: $(source_files) $(include_files)
	$(CC) $(CFLAGS) bptree.cpp -o $@

zip: $(source_files) $(include_files) makefile
	zip -r $(zip_name) $(source_files) $(include_files) makefile

clean: 
	rm -f tree $(zip_name)