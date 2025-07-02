Hello,

This is my first ever OS, made with a combination of a lot of different tutorials, and code segments written by people 4-10 years ago. I got interested in this project ever
since I took an Intro to Computer Systems course at my university, which introduced me to low level programming through the LC-3 (Little Computer 3) and the Von Neumann Model. I
would highly recommend checking out the book Introduction to Computing Systems by Patt and Patel. They do an excellent job of explaining things.
To learn about stuff, I am using a combination of the OSDev Wiki, Reddit posts (r/osdev), Comet Book (Operating Systems: Three Easy Pieces), Dinosaur Book (Operating Systems
Concepts 10th Edition), and anything else I can find on Google (the Stanford x86 cheat-sheet was very helpful).  


This 32-bit OS has, as of writing this:
1. A custom legacy bootloader that switches from real mode to protected mode
2. Sets up its own Global Descriptor Table
3. Performs a two-stage boot process to load the kernel into its proper memory position (0x100000) from disk via CHS addressing
4. Sets up its own Interrupt Descriptor Table, along with the various things that come with interrupt handling, like 8259 PIC 
5. Partially implements various parts of the C standard library, like stdint and stdio, (printf).
6. Sets up a Physical Memory Manager via a bitmap (in the future I might switch to a more optimized version, like a stack pmm, or buddy-buddy allocator)
7. Sets up a Virtual Memory Manager & enables paging
8. Maps the higher half kernel to 0xC0000000 (3GB), and runs the kernel there via trampoline code
