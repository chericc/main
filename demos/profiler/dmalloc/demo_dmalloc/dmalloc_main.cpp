#include "dmalloc_main.h"

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include "dmalloc.h"

#define xdebug(x...) do {printf("[debug][%s %d %s]", \
	__FILE__,__LINE__,__FUNCTION__);printf(x);} while (0)

#define FILE_TRIGGER_LOG_UNFREED "/tmp/malloc_unfreed"
#define FILE_LOG_STATS "/tmp/log_stats.txt"

static int break_flag = false;
static DmallocImp imp;

static void  my_track(const char *file, const unsigned int line,
				 const int func_id,
				 const DMALLOC_SIZE byte_size,
				 const DMALLOC_SIZE alignment,
				 const DMALLOC_PNT old_addr,
				 const DMALLOC_PNT new_addr)
{
    static pthread_mutex_t mutex_my_track = PTHREAD_MUTEX_INITIALIZER;
    static unsigned int uSizeIntrested = -1;
    static unsigned int uCountIntrested = 1;
    static unsigned int uCount = 0;

    if (byte_size == (DMALLOC_SIZE)uSizeIntrested)
    {
        ++uCount;
    }

    if (uCount == uCountIntrested)
    {
        pthread_mutex_lock (& mutex_my_track);

        printf ("\n\n\n**************   break: size=%u,count=%u   **************\n\n\n", uSizeIntrested, uCountIntrested);

        char *p = nullptr;
        *p = 0;

        pthread_mutex_unlock (& mutex_my_track);
    }

    return ;
}

static void main_dump_states ()
{
    static FILE *fp = NULL;

    DMALLOC_PNT heap_low = nullptr;
    DMALLOC_PNT heap_high = nullptr;
    unsigned long total_space = 0;
    unsigned long user_space = 0;
    unsigned long current_alloc = 0;
    unsigned long current_alloc_n = 0;
    unsigned long max_allocated = 0;
    unsigned long max_allocated_n = 0;
    unsigned long max_allocated_one = 0;

    dmalloc_get_stats(
        & heap_low, 
        & heap_high, 
        & total_space, 
        & user_space,
        & current_alloc, 
        & current_alloc_n, 
        & max_allocated, 
        & max_allocated_n, 
        & max_allocated_one
    );
    
    if (NULL == fp)
    {
        fp = fopen (FILE_LOG_STATS, "w");
    }
    if (NULL != fp)
    {
        char str_now[64] = {};
        time_t tNow = time(NULL);
        struct tm sTm = {};
        localtime_r (& tNow, & sTm);
        strftime (str_now, sizeof(str_now), "%F %T", & sTm);

        fprintf (fp, "[%s] alloc:[low=%p,high=%p,total=%lu,user=%lu,cur=%lu,curn=%lu,max=%lu,maxn=%lu,maxone=%lu]\n",
            str_now,
            heap_low,
            heap_high,
            total_space,
            user_space,
            current_alloc,
            current_alloc_n,
            max_allocated,
            max_allocated_n,
            max_allocated_one);
        fflush (fp);
        xdebug ("[%s] alloc:[low=%p,high=%p,total=%lu,user=%lu,cur=%lu,curn=%lu,max=%lu,maxn=%lu,maxone=%lu]\n",
            str_now,
            heap_low,
            heap_high,
            total_space,
            user_space,
            current_alloc,
            current_alloc_n,
            max_allocated,
            max_allocated_n,
            max_allocated_one);
    }
}

static void main_dmalloc_test()
{
    while (true)
    {
        sleep (1);

        char file_unfreed[] = FILE_TRIGGER_LOG_UNFREED;

        if (break_flag)
        {
            break;
        }

        if (access(file_unfreed, F_OK) == 0)
        {
            unlink (file_unfreed);
            dmalloc_message("-------------- log unfreed begin -------------\n");
            dmalloc_log_unfreed();
            dmalloc_message("-------------- log unfreed end -------------\n");
            sync();
        }

        main_dump_states ();
    }
}

static void *trd_dmalloc_imp (void *arg)
{
    dmalloc_track (my_track);

    main_dmalloc_test ();

    return nullptr;
}

DmallocImp::DmallocImp()
    : thread__(trd_dmalloc_imp, nullptr)
{
    xdebug ("DmallocImp::construction\n");
}

DmallocImp::~DmallocImp()
{
    xdebug ("DmallocImp::destruction\n");

    break_flag = true;
    if (thread__.joinable())
    {
        thread__.join();
    }
}