# gperftools

## demo

```c++
// test.cpp
int g_a = 0;

void fun3()
{
    for (int i = 0; i < 50; ++i)
    {
        g_a += (i % 5);
    }
}

void fun2()
{
    for (int i = 0; i < 100; ++i)
    {
        g_a += (i % 5);
    }
}

void fun1()
{
    for (int i = 0; i < 1 * 1000 * 1000; ++i)
    {
        g_a += (i % 5);

        fun2();
        fun3();
    }
}

int main()
{
    fun1();

    return g_a;
}
```

## steps(without libunwind)

```bash
# how to build gperftools?
# refer to /README.md

g++ test.cpp -c -g -o test.o
g++ test.o -o test.out 
LD_PRELOAD=/home/test/opensrc/gperftools/build/output/lib/libprofiler.so CPUPROFILE=/tmp/prof.out ./test.out

/home/test/opensrc/gperftools/build/output/bin/pprof-symbolize --help
/home/test/opensrc/gperftools/build/output/bin/pprof-symbolize --text ./test.out /tmp/prof.out
/home/test/opensrc/gperftools/build/output/bin/pprof-symbolize --svg ./test.out /tmp/prof.out > output.svg
```
