Hello,

This is my first ever OS, made with a combination of a lot of different tutorials, and code segments written by people 4-10 years ago. I got interested in this project ever
since I took an Intro to Computer Systems course at my university, which introduced me to low level programming through the LC-3 (Little Computer 3) and the Von Neumann Model. I
would highly recommend checking out the book Introduction to Computing Systems by Patt and Patel. They do an excellent job of explaining things.
To learn about stuff, I am using a combination of the OSDev Wiki, Reddit posts (r/osdev), Comet Book (Operating Systems: Three Easy Pieces), Dinosaur Book (Operating Systems
Concepts 10th Edition), and anything else I can find on Google (the Stanford x86 cheat-sheet was very helpful).  

This OS has, as of writing this:
1. A custom bootloader that switches from real mode to protected mode,
2. Sets up the Global Descriptor Table
3. Reads the disk through CHS addressing and loads the kernel into memory through a two-stage process (loads kernel to 0x10000 first, then loads kernel into 0x100000)
