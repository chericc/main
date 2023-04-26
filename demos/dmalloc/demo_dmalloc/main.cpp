#include <stdio.h>

#include <thread>
#include <chrono>
#include <string>
#include <map>
#include <iostream>

#include <stdlib.h>  
#include <unistd.h>     

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>

#include <pthread.h>

#define LEAK_SIZE_BIG (1 * 1024 * 1024)
#define LEAK_SIZE_SMALL (1024)

#define FILE_TO_OPEN    "/dev/null"
#define DIR_TO_OPEN     "."

struct BigMem
{
    unsigned char mem[LEAK_SIZE_BIG];
};

struct SmallMem
{
    unsigned char mem[LEAK_SIZE_BIG];
};

struct LeakInfo
{
    void(*func)();
    std::string desc;
};

void leak_malloc_big()
{
    void *p = malloc(LEAK_SIZE_BIG);
    printf("%p\n", p);
}

void leak_malloc_small()
{
    void *p = malloc(LEAK_SIZE_SMALL);
    printf("%p\n", p);
}

void leak_new_big()
{
    new(BigMem);
}

void leak_new_small()
{
    new(SmallMem);
}

void leak_newa_big()
{
    new(unsigned char[LEAK_SIZE_BIG]);
}

void leak_newa_small()
{
    new(unsigned char[LEAK_SIZE_SMALL]);
}

void leak_open()
{
    int fd = open(FILE_TO_OPEN, O_RDONLY);
    if (fd < 0)
    {
        printf("open failed\n");
    }
}

void *thd_proc_example(void *)
{
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void leak_pthread()
{
    pthread_t tid{};
    int ret = 0;

    ret = pthread_create(&tid, nullptr, thd_proc_example, nullptr);
    if (ret != 0)
    {
        printf("pthread_create failed\n");
    }
}

void leak_fopen()
{
    FILE *fp = nullptr;
    fp = fopen(FILE_TO_OPEN, "r");
    if (nullptr == fp)
    {
        printf("fopen failed\n");
    }
}

void leak_opendir()
{
    DIR *dirp = nullptr;
    dirp = opendir(DIR_TO_OPEN);
    if (nullptr == dirp)
    {
        printf("open dir failed\n");
    }
}

int main(int argc, char *argv[])
{
    int opt = 0;

    std::map<char, LeakInfo> leak_map;
    std::string optstr;
    int print_usage = 0;

    leak_map['a'] = LeakInfo{leak_malloc_big,"leak_malloc_big"};
    leak_map['b'] = LeakInfo{leak_malloc_small,"leak_malloc_small"};
    leak_map['c'] = LeakInfo{leak_new_big,"leak_new_big"};
    leak_map['d'] = LeakInfo{leak_new_small,"leak_new_small"};
    leak_map['e'] = LeakInfo{leak_newa_big,"leak_newa_big"};
    leak_map['f'] = LeakInfo{leak_newa_small,"leak_newa_small"};
    leak_map['g'] = LeakInfo{leak_open,"leak_open"};
    leak_map['h'] = LeakInfo{leak_pthread,"leak_pthread"};
    leak_map['i'] = LeakInfo{leak_fopen,"leak_fopen"};
    leak_map['j'] = LeakInfo{leak_opendir,"leak_opendir"};

    for (auto const& it : leak_map)
    {
        char str_tmp[2] = {it.first,'\0'};
        optstr.append(str_tmp);
    }

    auto item = leak_map.cbegin();

    if (argc <= 1)
    {
        print_usage = true;
    }
    else
    {
        while ((opt = getopt(argc, argv, optstr.c_str())) != -1)
        {
            item = leak_map.find(opt);
            if (item != leak_map.cend())
            {
                break;
            }
            else 
            {
                print_usage = true;
            }
        }
    }

    if (print_usage)
    {
        printf("usage:\n");
        for (auto const& it : leak_map)
        {
            printf("%c:%s\n", it.first, it.second.desc.c_str());
        }
        exit(0);
    }
    
    while (true)
    {
        printf("cicling\n");
        item->second.func();
        
        if ('q' == getchar())
        {
            printf("break\n");
            break;
        }
    }

    return 0;
}