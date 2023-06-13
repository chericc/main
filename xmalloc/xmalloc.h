#pragma once

typedef void * xmalloc_ptr;
typedef unsigned long xmalloc_size_t;
typedef void xmalloc_free_ret;

xmalloc_ptr malloc(xmalloc_size_t size);
xmalloc_ptr calloc(xmalloc_size_t nb_elems, xmalloc_size_t size);
xmalloc_ptr realloc(xmalloc_ptr oldptr, xmalloc_size_t newsize);
xmalloc_ptr recalloc(xmalloc_ptr oldptr, xmalloc_size_t newsize);
xmalloc_free_ret free(xmalloc_ptr ptr);

char *strdup(const char *str);
char *strndup(const char *str, xmalloc_size_t max_len);

void xmalloc_message(const char *format, ...);
xmalloc_ptr xmalloc_malloc(xmalloc_size_t size);
xmalloc_ptr xmalloc_calloc(xmalloc_size_t nb_elems, xmalloc_size_t size);
xmalloc_ptr xmalloc_realloc(xmalloc_ptr oldptr, xmalloc_size_t newsize);
xmalloc_ptr xmalloc_recalloc(xmalloc_ptr oldptr, xmalloc_size_t newsize);
xmalloc_free_ret xmalloc_free(xmalloc_ptr ptr);