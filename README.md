Hello,

This is my first ever OS, made with a combination of a lot of different tutorials, code segments written by other people, and some of my own coding. The goal of this project was to really just to learn about low-level
stuff, and to build a monolithic kernel (i.e., memory management, interrupts, file system, drivers, etc. all in one kernel.). There are numerous resources I used to learn about OS development and whatnot, some of the
primary and most used resources include OSTEP, (https://pages.cs.wisc.edu/~remzi/OSTEP/), the OSDev wiki, (https://wiki.osdev.org/Expanded_Main_Page), Stack Overflow, and a bunch of different OS's made by other people. 


As of right now, this OS has:
1. A multi-stage custom legacy bootloader that uses LBA addressing to load files from disk onto memory
2. A Global Descriptor Table
3. An Interrupt Descriptor Table & the 8259 PIC set up
4. Parts of C libraries implemented, (stdlib, stdio, unistd, stdint, etc..)
5. A Physical Memory Manager (using the bitmap approach)
6. A Virtual Memory Manager & paging enabled
7. A File System (vsfs, check OSTEP chapter 40, "File System Implementation")
8. Trampoline code to run higher half kernel at 0xC0000000 (3GB)
9. A shell that allows various commands to be run by the user, and allows the user to run their own raw bin files in user mode

GOALS FOR THE FUTURE:
1. Implement a basic working scheduler (Round-Robin approach likely, check OSTEP chapter 7, section 7)
2. Rewrite printf entirely (doesn't work with user mode, I need it work with user mode so I can make better programs)
3. Write better programs for the user to be able to use
4. Proper userspace management (i.e., creating separate page tables & directory for user processes)
5. C standard libraries
6. maybe try to make the OS follow the POSIX standard? Need to do more research on it
7. Implement threads & processes 
8. Implement an ELF file loader
9. Switch to a better file system (create drivers for ext4 or FAT32)
10. implement I/O standard streams (stdout, stderr, stdin)
11. Implement concurrency-related things, (like semaphores, locks, conditional variables)
12. Play around with VGA graphical memory (currently just using text-mode, 0xB800)
13. Networking (have done 0 research on this)
14. Refactor to 64-bit and use UEFI (again, 0 research)
