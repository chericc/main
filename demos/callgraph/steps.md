# callgraph

## compile

```bash
mkdir build
cd build
g++ -c -finstrument-functions -fprofile-abs-path -g ../test.cpp -o test.o
g++ test.o -g -o test.out
```

## run

```bash
./test.out
enter: 0x5562f022f254 call 0x7fdeea3bed90
enter: 0x5562f022f1b5 call 0x5562f022f286
leave: 0x5562f022f1b5 call 0x5562f022f286
enter: 0x5562f022f205 call 0x5562f022f298
leave: 0x5562f022f205 call 0x5562f022f298
leave: 0x5562f022f254 call 0x7fdeea3bed90
```

## how to map vir-address into offset?

```c
// test.c
#include <stdio.h>
int main()
{
        printf("main=%p\n", main);
        while (1)
        {
        }
        return 0;
}
```

```bash
gcc test.c

./a.out
main=0x557e12c28149

cat /proc/16578/maps
557e12c27000-557e12c28000 r--p 00000000 08:02 1311032                    /home/test/tmp/a.out
557e12c28000-557e12c29000 r-xp 00001000 08:02 1311032                    /home/test/tmp/a.out
557e12c29000-557e12c2a000 r--p 00002000 08:02 1311032                    /home/test/tmp/a.out
557e12c2a000-557e12c2b000 r--p 00002000 08:02 1311032                    /home/test/tmp/a.out
557e12c2b000-557e12c2c000 rw-p 00003000 08:02 1311032                    /home/test/tmp/a.out
557e13079000-557e1309a000 rw-p 00000000 00:00 0                          [heap]

offset = 0x557e12c28149 - 0x557e12c27000 = 0x1149

objdump -t a.out
...
0000000000001149 g     F .text  0000000000000028              main
...
```

也即，函数的虚拟地址是文件的加载地址和偏移地址之和。