CFLAGS=-Wall -pthread

mmu:	mmu.c
	cc $(CFLAGS) -o mmu mmu.c


clean:
	rm -f mmu