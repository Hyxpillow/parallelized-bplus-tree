CC := g++
CFLAGS := 
OBJ := page.o bptree.o

src_dir := .
include_dir := .

tree: $(OBJ)
	$(CC) $(CFLAGS) $^ -o $@

%.o: $(src_dir)/%.c
	$(CC) -c $(CFLAGS) $^ -o $@ -I $(include_dir)

clean: 
	rm -f tree *.o