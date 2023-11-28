CC := g++
CFLAGS := -std=c++17 -fopenmp -fsanitize=thread -g
OBJ := page.o bptree.o

src_dir := .
include_dir := .

tree: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: $(src_dir)/%.cpp
	$(CC) -c $(CFLAGS) $^ -o $@ -I $(include_dir)

clean: 
	rm -f tree *.o