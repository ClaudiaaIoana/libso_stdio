CC := gcc
ASM := my_open.S my_read.S my_write.S my_close.S

main: main.c $(ASM)
	$(CC) -m64 -o main main.c $(ASM)

clean:
	rm -f main