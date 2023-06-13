#pragma once

typedef struct skip_alloc_st 
{
    unsigned char sa_flags;

    unsigned int sa_user_space;
    unsigned int sa_total_size;
} skip_alloc_t;