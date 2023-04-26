# ELF

## 1 Overview

ref:

```
https://www.baeldung.com/linux/executable-and-linkable-format-file
https://linux-audit.com/elf-binaries-on-linux-understanding-and-analysis/
```

ELF is short for Executable and Linkable Format. It's a format used for storing binaries, libraries, and core dumps on disks in Linux and Unix-based systems.

## 2 The Structure of the ELF File

The ELF file is divided into two parts. The first part is the **ELF header**, while the second is the **file data**.

The file data is made up of the **Program header table**, **Section header table**, and **Data**.

ELF header is always available in the ELF file. The Section header table is important during link time to create an executable. The program header table is useful during runtime to help load the executable into memory.

```
ELF file structure:

-------
ELF header
Program header table
.text
.rodata
....
.data
Section header table
-------
```

### 2.1 ELF header

ELF header is found at the start of the file. It contains metadata about the file.

For example, some of the metadata includes information about whether the ELF file is 32bit or 64bit, whether it's using little-endian or big-endian, the ELF version, and the architecture that the file requires.

In particular, the metadata helps different processor architectures to interpret the ELF file.

```C
# cat test.c
int a = 0;
int b = 0;
int c = 0;

void main()
{
	a = b = c;
}

# gcc -c test.c
# gcc test.o

# readelf -h test.o
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              REL (Relocatable file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x0
  Start of program headers:          0 (bytes into file)
  Start of section headers:          800 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           0 (bytes)
  Number of program headers:         0
  Size of section headers:           64 (bytes)
  Number of section headers:         13
  Section header string table index: 12

# readelf -h a.out
ELF Header:
  Magic:   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  Type:                              DYN (Shared object file)
  Machine:                           Advanced Micro Devices X86-64
  Version:                           0x1
  Entry point address:               0x1040
  Start of program headers:          64 (bytes into file)
  Start of section headers:          14680 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         13
  Size of section headers:           64 (bytes)
  Number of section headers:         29
  Section header string table index: 28
```

### 2.2 ELF Header Details

| Field                             | Explanation                                                  |
| :-------------------------------- | :----------------------------------------------------------- |
| Type                              | The value in this field specifies the object file type. For instance, 1 is for relocatable, 2 is for an excutable, 3 is for a shared object, and 4 is for a core file. |
| Entry point address               | This indicates the address where the program should start executing. In the case that the file is not an executable file, the value in this field is set to 0. |
| Start of program  headers         | This is the offset on the file where the program headers start. |
| Section header string table index | The section table index of the encry representing the section name string table. |

### 2.3 Program Header Table(Segments)

An ELF file consists of zero or more segments, and **describe how to create a process/memory image for runtime execution**. When the kernel sees these segments, it uses them to map them into virtual address space, using the mmap system call. In other words, it converts predefined instructions into a memory image. If your ELF file is a normal binary, it requires these program headers. Otherwise, it simply won't run. It uses these headers, with the underlying data structure, to form a process. This process is similar for shared libraries.

```bash
# readelf -l a.out 

Elf file type is DYN (Shared object file)
Entry point 0x1040
There are 13 program headers, starting at offset 64

Program Headers:
  Type           Offset             VirtAddr           PhysAddr
                 FileSiz            MemSiz              Flags  Align
  PHDR           0x0000000000000040 0x0000000000000040 0x0000000000000040
                 0x00000000000002d8 0x00000000000002d8  R      0x8
  INTERP         0x0000000000000318 0x0000000000000318 0x0000000000000318
                 0x000000000000001c 0x000000000000001c  R      0x1
      [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]
  LOAD           0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x00000000000005c8 0x00000000000005c8  R      0x1000
  LOAD           0x0000000000001000 0x0000000000001000 0x0000000000001000
                 0x00000000000001d5 0x00000000000001d5  R E    0x1000
  LOAD           0x0000000000002000 0x0000000000002000 0x0000000000002000
                 0x0000000000000130 0x0000000000000130  R      0x1000
  LOAD           0x0000000000002df0 0x0000000000003df0 0x0000000000003df0
                 0x0000000000000220 0x0000000000000230  RW     0x1000
  DYNAMIC        0x0000000000002e00 0x0000000000003e00 0x0000000000003e00
                 0x00000000000001c0 0x00000000000001c0  RW     0x8
  NOTE           0x0000000000000338 0x0000000000000338 0x0000000000000338
                 0x0000000000000020 0x0000000000000020  R      0x8
  NOTE           0x0000000000000358 0x0000000000000358 0x0000000000000358
                 0x0000000000000044 0x0000000000000044  R      0x4
  GNU_PROPERTY   0x0000000000000338 0x0000000000000338 0x0000000000000338
                 0x0000000000000020 0x0000000000000020  R      0x8
  GNU_EH_FRAME   0x0000000000002004 0x0000000000002004 0x0000000000002004
                 0x000000000000003c 0x000000000000003c  R      0x4
  GNU_STACK      0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x0000000000000000 0x0000000000000000  RW     0x10
  GNU_RELRO      0x0000000000002df0 0x0000000000003df0 0x0000000000003df0
                 0x0000000000000210 0x0000000000000210  R      0x1

 Section to Segment mapping:
  Segment Sections...
   00     
   01     .interp 
   02     .interp .note.gnu.property .note.gnu.build-id .note.ABI-tag .gnu.hash .dynsym .dynstr .gnu.version .gnu.version_r .rela.dyn 
   03     .init .plt .plt.got .text .fini 
   04     .rodata .eh_frame_hdr .eh_frame 
   05     .init_array .fini_array .dynamic .got .data .bss 
   06     .dynamic 
   07     .note.gnu.property 
   08     .note.gnu.build-id .note.ABI-tag 
   09     .note.gnu.property 
   10     .eh_frame_hdr 
   11     
   12     .init_array .fini_array .dynamic .got
```

**GNU_EH_FRAME**

This is a sorted queue used by the GNU C compiler. It stores exception handlers. So when something goes wrong, it can use this area to deal correctly with it.

**GNU_STACK**

This header is used to store stack information. The stack is a buffer, or scratch place, where items are stored, like local variables. This will occur with LIFO, similar to putting boxes on top of each other. When a process function is started a block is reserved. When the function is finished, it will be marked as free again. Now the interesting part is that a stack shouldn't be executable, as this might introduce security vulnerabilities. By manipulation of memory, one could refer to this executable stack and run intended instructions. 

 

### 2.4 Section Header Table(Sections)

The section header stores information about sections. This information is used during dynamic link time, just before the program is executed.Sections is used for categorizing instructions and data.

A linker links the binary file with shared libraries that it needs by loading them into memory. The linker's implementation is specific to the operation system.

Additionally, the section header table contains information that's used by other files to find the symbolic and references of the program.

```bash
# readelf -S test.o 
There are 13 section headers, starting at offset 0x320:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .text             PROGBITS         0000000000000000  00000040
       0000000000000023  0000000000000000  AX       0     0     1
  [ 2] .rela.text        RELA             0000000000000000  00000238
       0000000000000060  0000000000000018   I      10     1     8
  [ 3] .data             PROGBITS         0000000000000000  00000063
       0000000000000000  0000000000000000  WA       0     0     1
  [ 4] .bss              NOBITS           0000000000000000  00000064
       000000000000000c  0000000000000000  WA       0     0     4
  [ 5] .comment          PROGBITS         0000000000000000  00000064
       000000000000002c  0000000000000001  MS       0     0     1
  [ 6] .note.GNU-stack   PROGBITS         0000000000000000  00000090
       0000000000000000  0000000000000000           0     0     1
  [ 7] .note.gnu.propert NOTE             0000000000000000  00000090
       0000000000000020  0000000000000000   A       0     0     8
  [ 8] .eh_frame         PROGBITS         0000000000000000  000000b0
       0000000000000038  0000000000000000   A       0     0     8
  [ 9] .rela.eh_frame    RELA             0000000000000000  00000298
       0000000000000018  0000000000000018   I      10     8     8
  [10] .symtab           SYMTAB           0000000000000000  000000e8
       0000000000000138  0000000000000018          11     9     8
  [11] .strtab           STRTAB           0000000000000000  00000220
       0000000000000011  0000000000000000           0     0     1
  [12] .shstrtab         STRTAB           0000000000000000  000002b0
       000000000000006c  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  l (large), p (processor specific)
  
# readelf -S a.out 
There are 29 section headers, starting at offset 0x3958:

Section Headers:
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .interp           PROGBITS         0000000000000318  00000318
       000000000000001c  0000000000000000   A       0     0     1
  [ 2] .note.gnu.propert NOTE             0000000000000338  00000338
       0000000000000020  0000000000000000   A       0     0     8
  [ 3] .note.gnu.build-i NOTE             0000000000000358  00000358
       0000000000000024  0000000000000000   A       0     0     4
  [ 4] .note.ABI-tag     NOTE             000000000000037c  0000037c
       0000000000000020  0000000000000000   A       0     0     4
  [ 5] .gnu.hash         GNU_HASH         00000000000003a0  000003a0
       0000000000000024  0000000000000000   A       6     0     8
  [ 6] .dynsym           DYNSYM           00000000000003c8  000003c8
       0000000000000090  0000000000000018   A       7     1     8
  [ 7] .dynstr           STRTAB           0000000000000458  00000458
       000000000000007d  0000000000000000   A       0     0     1
  [ 8] .gnu.version      VERSYM           00000000000004d6  000004d6
       000000000000000c  0000000000000002   A       6     0     2
  [ 9] .gnu.version_r    VERNEED          00000000000004e8  000004e8
       0000000000000020  0000000000000000   A       7     1     8
  [10] .rela.dyn         RELA             0000000000000508  00000508
       00000000000000c0  0000000000000018   A       6     0     8
  [11] .init             PROGBITS         0000000000001000  00001000
       000000000000001b  0000000000000000  AX       0     0     4
  [12] .plt              PROGBITS         0000000000001020  00001020
       0000000000000010  0000000000000010  AX       0     0     16
  [13] .plt.got          PROGBITS         0000000000001030  00001030
       0000000000000010  0000000000000010  AX       0     0     16
  [14] .text             PROGBITS         0000000000001040  00001040
       0000000000000185  0000000000000000  AX       0     0     16
  [15] .fini             PROGBITS         00000000000011c8  000011c8
       000000000000000d  0000000000000000  AX       0     0     4
  [16] .rodata           PROGBITS         0000000000002000  00002000
       0000000000000004  0000000000000004  AM       0     0     4
  [17] .eh_frame_hdr     PROGBITS         0000000000002004  00002004
       000000000000003c  0000000000000000   A       0     0     4
  [18] .eh_frame         PROGBITS         0000000000002040  00002040
       00000000000000f0  0000000000000000   A       0     0     8
  [19] .init_array       INIT_ARRAY       0000000000003df0  00002df0
       0000000000000008  0000000000000008  WA       0     0     8
  [20] .fini_array       FINI_ARRAY       0000000000003df8  00002df8
       0000000000000008  0000000000000008  WA       0     0     8
  [21] .dynamic          DYNAMIC          0000000000003e00  00002e00
       00000000000001c0  0000000000000010  WA       7     0     8
  [22] .got              PROGBITS         0000000000003fc0  00002fc0
       0000000000000040  0000000000000008  WA       0     0     8
  [23] .data             PROGBITS         0000000000004000  00003000
       0000000000000010  0000000000000000  WA       0     0     8
  [24] .bss              NOBITS           0000000000004010  00003010
       0000000000000010  0000000000000000  WA       0     0     4
  [25] .comment          PROGBITS         0000000000000000  00003010
       000000000000002b  0000000000000001  MS       0     0     1
  [26] .symtab           SYMTAB           0000000000000000  00003040
       0000000000000618  0000000000000018          27    44     8
  [27] .strtab           STRTAB           0000000000000000  00003658
       00000000000001f2  0000000000000000           0     0     1
  [28] .shstrtab         STRTAB           0000000000000000  0000384a
       000000000000010c  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),
  L (link order), O (extra OS processing required), G (group), T (TLS),
  C (compressed), x (unknown), o (OS specific), E (exclude),
  l (large), p (processor specific)
```

Section headers are significant at link time to link the executable with the libraries it needs to run successfully.

### 2.5 .text

This section holds the instructions that the program needs for it to run.

### 2.6 .rodata and .data

.radata stands for read-only data. As such, these sections contain the actual, initialized data, which the program will need in memory. The memory reserves more space for the data segment than specified in the ELF file to make room for uninitialized viriables.

