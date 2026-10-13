# 📄 Demand Paging Simulator — Operating Systems Fundamentals

<p>
  <img alt="C" src="https://img.shields.io/badge/C-1f2328?style=flat-square&logo=c&logoColor=white">
  <img alt="Course" src="https://img.shields.io/badge/Course-Operating%20Systems%20Fundamentals%20(FSO)-1f2328?style=flat-square">
  <img alt="Year" src="https://img.shields.io/badge/2023%2F24-2nd%20year%2C%201st%20semester-1f2328?style=flat-square">
</p>

Individual assignment of *Fundamentos de Sistemas de Operação* (Operating Systems Fundamentals), Computer Science degree at **NOVA School of Science and Technology (FCT NOVA)**.

## About

A simulator of how an operating system manages memory with **on-demand paging**. It reads a trace of memory accesses made by a process, translates each virtual address into a physical one like the MMU does (8 KB pages, 32-bit addresses), and simulates the OS handling the exceptions:

- **Invalid pages** (like page 0, the classic null pointer) "kill" the process with a segmentation fault.
- **Page faults** load the page into a free frame or, when memory is full, pick a **victim page** to swap out.

The provided simulator did the address translation; I implemented the OS side: page-fault handling, a **clock-style replacement policy** (a circular pointer over the frames with reference bits that give recently loaded pages a second chance), and the statistics.

## Running it

Needs a C compiler and `make`.

```bash
make
./mmu <number of frames> <trace file>
```

Each trace line is a hexadecimal virtual address and `R` or `W`. With 4 frames:

```text
$ ./mmu 4 ex1.trace
...
swapout page 10 from frame 1
swapin page 8 to frame 1
Memory accesses: 9
Page faults: 6 (66.67% of memory accesses)
Swap outs: 2
```

On a real program's trace (1 million accesses from `bzip`, not included here because it's 11 MB) with 64 frames, only 0.10% of accesses cause a page fault.

## What I learned

- Address translation with page tables and what the MMU does in hardware vs. what the OS does
- Page-fault handling and page replacement policies (FIFO, clock / second chance)
- Measuring how the number of frames affects the page-fault rate

## Credits

**Miguel Carmo** ([@MCCProgrammer](https://github.com/MCCProgrammer))

The simulator framework in `mmu.c` (address translation, trace reading, the `Makefile`) and the trace files were provided by the FSO teaching staff (Vitor Duarte); the page-fault handling and replacement policy were implemented by me.

---

<sub>The assignment statement is not included in this repository, as it belongs to the course's teaching staff. This is a student project shared for portfolio purposes.</sub>
