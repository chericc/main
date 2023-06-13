#pragma once

int xmalloc_chunk_startup();

void *xmalloc_chunk_malloc(unsigned long size);
int xmalloc_chunk_free(void *ptr);
void *xmalloc_chunk_realloc(void *oldptr, unsigned long new_size);
