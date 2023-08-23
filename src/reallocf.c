#include "malloc.h"

void    *ft_reallocf(void *ptr, size_t size)
{
    void    *new_block;

    pthread_mutex_lock(&g_mutex);
    new_block = ft_realloc(ptr, size);
    if (new_block == NULL && ptr)
        ft_free(ptr);
    pthread_mutex_unlock(&g_mutex);
    return (new_block);
}