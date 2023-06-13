#include "xmalloc.h"

#include "def.h"

static int s_b_started = 0;

static int xmalloc_in()
{
    if (!s_b_started)
    {
        if (!xmalloc_startup())
        {
            return xmalloc_false;
        }
    }
    return xmalloc_true;
}

int xmalloc_startup()
{
    return 0;
}

xmalloc_ptr malloc(xmalloc_size_t size)
{
    return xmalloc_malloc(size);
}

xmalloc_ptr calloc(xmalloc_size_t nb_elems, xmalloc_size_t size)
{
    return xmalloc_calloc(nb_elems, size);
}

xmalloc_ptr realloc(xmalloc_ptr oldptr, xmalloc_size_t newsize)
{
    return xmalloc_realloc(oldptr, newsize);
}

xmalloc_ptr recalloc(xmalloc_ptr oldptr, xmalloc_size_t newsize)
{
    return xmalloc_recalloc(oldptr, newsize);
}

xmalloc_free_ret free(xmalloc_ptr ptr)
{
    return xmalloc_free(ptr);
}

char *strdup(const char *str)
{
    ;
}

char *strndup(const char *str, xmalloc_size_t max_len)
{

}

void xmalloc_message(const char *format, ...)
{

}

xmalloc_ptr xmalloc_malloc(xmalloc_size_t size)
{
    if (!xmalloc_in())
    {
        return xmalloc_nullptr;
    }

    
}

xmalloc_ptr xmalloc_calloc(xmalloc_size_t nb_elems, xmalloc_size_t size)
{

}

xmalloc_ptr xmalloc_realloc(xmalloc_ptr oldptr, xmalloc_size_t newsize)
{

}

xmalloc_ptr xmalloc_recalloc(xmalloc_ptr oldptr, xmalloc_size_t newsize)
{

}

xmalloc_free_ret xmalloc_free(xmalloc_ptr ptr)
{

}