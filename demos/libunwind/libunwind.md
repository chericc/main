# libunwind

## src

```c++
#include <stdio.h>

#define UNW_LOCAL_ONLY

#include "libunwind.h"

void func_backtrace(void)
{
    unw_context_t ctx = {0};
    unw_cursor_t cursor = {0};
    unw_word_t offset = 0;
    unw_word_t pc = 0;
    char func_name[1024] = {0};
    int32_t ret = 0;

    printf("\n----------------------stack backtrace----------------------------\n");

    ret = unw_getcontext(&ctx);
    if(0 != ret)
    {
        printf("get context failed\n");
        return;
    }

    ret = unw_init_local(&cursor, &ctx);
    if(0 != ret)
    {
        printf("init local cursor failed\n");
        return;
    }

    while(0 < unw_step(&cursor))
    {
        ret = unw_get_proc_name(&cursor, func_name, sizeof(func_name), &offset);
        if(0 != ret)
        {
            printf("can not get func name\n");
        }
        else
        {
            unw_get_reg(&cursor, UNW_REG_IP, &pc);
            printf("0x%lx:(%s+0x%lx)\n", pc, func_name, offset);
        }
    }
}

void fun()
{
    func_backtrace();
}

int main()
{
    fun();
    return 0;
}
```

## steps

```bash
g++ -I/home/test/opensrc/libunwind/build/output/include -c test.cpp -g -o test.o 
g++ test.o -L/home/test/opensrc/libunwind/build/output/lib/ -lunwind -o test.out
LD_PRELOAD=/home/test/opensrc/libunwind/build/output/lib/libunwind.so ./test.out
```