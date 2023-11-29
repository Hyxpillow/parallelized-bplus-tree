CC := g++
CFLAGS := -std=c++17 -fopenmp -fsanitize=thread -g
OBJ := bptree.o

src_dir := .
include_dir := .

tree: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: $(src_dir)/%.c
	$(CC) -c $(CFLAGS) $^ -o $@ -I $(include_dir)

clean: 
	rm -f tree *.o