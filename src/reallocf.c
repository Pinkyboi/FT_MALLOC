#include "malloc.h"

void    *ft_reallocf(void *ptr, size_t size)
{
    void    *new_ptr;

    pthread_mutex_lock(&g_mutex);
    new_ptr = ft_realloc(ptr, size);
    if (new_ptr == NULL && ptr)
        ft_free(ptr);
    pthread_mutex_unlock(&g_mutex);
    return (new_ptr);
}