
/**
 *   mmu.c - simulator of a MMU and the OS memory management for on-demand paging 
 *          with page replacement and swap space.
 *  FSO - LEI
 *
 *  2023 Vitor Duarte
 *  DI - FCT UNL
 */

#include <stdio.h>
#include <stdlib.h>

/*
Using:
 - CPU with 32 bits for virtual addresses and real addresses 
 - pages with 8KB (2^13) 
 - so maximum page table size is 2^32/2^13=2^19 (512K) entries
*/
#define PAGESIZE (8*1024)    // 8KBytes
#define OFFSET_BITS 13       // 2^13 = 8K
#define MAX_VIRTUAL_PAGES (512*1024)


/*
 * real memory
 */
#define MAX_REAL_PAGES (256)  // eg. 256 * 8K = 2 MBytes


/*
 * page table
 */
unsigned int pageTable[MAX_VIRTUAL_PAGES];  // not all entries need to be in use

// page "bits" to indicate that a page isn't mapped to physical memory
// or that a page isn't valit to this process
#define NOT_MAPPED  (-1)
#define NOT_VALID   (-2)

#define REFERENCED 1
#define NOT_REFERENCED 0

/*
 * frames table
 */
unsigned int frameTable[MAX_REAL_PAGES];  // max real memory
int numFrames;                            // real memory to simulate
unsigned int referencedBits[MAX_REAL_PAGES];
// physical memory frame is free
#define FREE  (-1)




/*
 *  statistics:
 */
int count;      // memory access counter
int pagefaults; // page faults counter
int nextFrame;
int swapouts;   // pages swapped out counter



/**
 * initTabs - OS work:
 *          init page table of a process
 *          init frame table representing the machine's physical memory
 */
void initTabs() {
    nextFrame=0;
    count=0;
    swapouts=0;
    pagefaults=0;
    for ( int i=0; i<MAX_REAL_PAGES; i++ ) 
        frameTable[i] = FREE; // not in use

    for ( int i=0; i<MAX_VIRTUAL_PAGES; i++)
        pageTable[i] = NOT_MAPPED;  // not mapped
    for (int i = 0; i < MAX_REAL_PAGES; i++) {
        referencedBits[i] = NOT_REFERENCED;  
    }
    pageTable[0] = NOT_VALID;       // not valid addresses (eg. address of NULL is here)
    pageTable[MAX_VIRTUAL_PAGES-1] = NOT_VALID; // not valid addresses
}
    


/**
 * os_pagein - load page contents to memory frame
 *          can be all zeros (if new heap page), can load from file,
 *          or can load from swap space the saved page
 */
void os_pagein(int page, int frame) {
    // just a simulation, so just print a message
    printf("swapin page %x to frame %x\n", page, frame);
}


/**
 * os_pageout - discard or save frame contents to secondary memory (swap space)
 *          to save page content
 */
void os_pageout(int page, int frame) {
    // just a simulation, so just print a message
    printf("swapout page %x from frame %x\n", page, frame);
    swapouts++;  // one more page out
}


/**
 * os_handle_invalidpage - handler to the invalid page reference exception (interrupt)
 *          usualy the OS will kill the processes
 */
void os_handle_invalidpage( unsigned page, unsigned addr ) {
    // OS  kills this process (maybe sending SIGVEC)
    // in this simulator just prints a message
    printf("ERROR: invalid address: %x (page %x)\n", addr, page);
}



/**
 * os_handle_pagefault - handler to the page fault reference exception (interrupt)
 *          usualy the OS will try to map a new frame to this page
 *          and load its contents (all zeros, from a file or from swap space) 
 */
void os_handle_pagefault( unsigned page ) {
    if (pageTable[page] == NOT_VALID) {
        os_handle_invalidpage(page, page * PAGESIZE);
    }
    //int currentPageFrame = pageTable[page];
    
    while (referencedBits[nextFrame]!=0)
    {
        referencedBits[nextFrame]=0;
        nextFrame = (nextFrame + 1) % numFrames;
    }
    referencedBits[nextFrame]=1;
    if(frameTable[nextFrame] == FREE){
        frameTable[nextFrame] = page;
        pageTable[page] = nextFrame;
        os_pagein(page, nextFrame); 
    }else{
        os_pageout(frameTable[nextFrame], nextFrame);
        pageTable[frameTable[nextFrame]] = NOT_MAPPED;
        frameTable[nextFrame] = page;
        pageTable[page] = nextFrame;
        os_pagein(page, nextFrame); 

    }
    pagefaults++;
    
}


/**
 * translateOneAddr - translate a virtual address to real address using pageTable
 *          simulate hardware MMU working
 *  returns physical address for the virtual addr
 */
unsigned int translateOneAddr(unsigned int addr) {
    int page;
    int frame;
    int realAddress=0;

    page = addr >> OFFSET_BITS;
    //printf("page:%x\n", page); // debug
    frame = pageTable[ page ];
    if ( frame == NOT_VALID ) {    // invalid page
        os_handle_invalidpage( page, addr );  // interruption to OS to handle
        return -1;  // same as 0xffffffffffff (max addr)
    }
    if ( frame == NOT_MAPPED ) {    // not mapped
        os_handle_pagefault( page );  // interruption to OS to handle
        frame = pageTable[ page ];
    }
    realAddress =  (frame << OFFSET_BITS) |
           (addr & ((1 << OFFSET_BITS) - 1)); /* real address */
           
    return realAddress;
}

/**
 * simulateAllSteps - simulate all memory acesses with translation from
 *          virtual address to real addresses
 *          tf is the name of the text file with addresses, one per line
 */
void simulateAllSteps(FILE *tf) {
    unsigned Vaddr, Raddr;
    char rw;

    while (fscanf(tf, "%x %c", &Vaddr, &rw) == 2) {
        Raddr = translateOneAddr(Vaddr); // translate vitual addr
        if ( Raddr == -1 ) {
            printf("Simulated process Segmentation fault!\n");    // abort program
            return;
        }
        count++;    // one more address accessed
    }
}


/************* MAIN **************/

int main(int argc, char *argv[]) {
    FILE *traceFile;
    char *prog = argv[0];

    if (argc != 3) {
        printf("usage: %s numb_frames memtrace\n", prog);
        return 1;
    }
    numFrames = strtol(argv[1], NULL, 10);
    if ( numFrames<=0 || numFrames >MAX_REAL_PAGES) {
        printf("usage: numb_frames must be in 1..%i\n", MAX_REAL_PAGES);
        return 1;
    }
    if ((traceFile = fopen(argv[2], "r")) == NULL) {
        printf("Trace-file '%s' error\n", argv[2]);
        return 1;
    }

    /* do it */
    initTabs();
    simulateAllSteps(traceFile);
    
    /* statistics */
    printf("Memory accesses: %d\n", count );
    printf("Page faults: %d (%.2f%% of memory accesses)\n",
            pagefaults, (float)pagefaults/count*100.0);
    printf("Swap outs: %d\n", swapouts );
    return 0;
}
