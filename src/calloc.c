#include "malloc.h"

static void *ft_memset(void *b, int c, size_t len)
{
    for (size_t i = 0; i < len; i++)
        ((char *)b)[i] = c;
    return (b);
}

void *ft_calloc(size_t count, size_t size)
{
    void *ptr;

    pthread_mutex_lock(&g_mutex);
    if ((ptr = ft_malloc(count * size)) == NULL)
        return (NULL);
    ft_memset(ptr, 0, count * size);
    pthread_mutex_unlock(&g_mutex);
    return (ptr);
}