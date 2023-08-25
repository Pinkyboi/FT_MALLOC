#include "malloc.h"

static void *ft_memset(void *b, int c, size_t len)
{
    for (size_t i = 0; i < len; i++)
        ((char *)b)[i] = c;
    return (b);
}

static void *ft_calloc(size_t count, size_t size)
{
    void *new_block;

    if ((new_block = ft_malloc(count * size)) == NULL)
        return (NULL);
    ft_memset(new_block, 0, count * size);
    return (new_block);
}

void *calloc(size_t count, size_t size)
{
    void *ptr;

    pthread_mutex_lock(&g_mutex);
    ptr = ft_calloc(count, size);
    pthread_mutex_unlock(&g_mutex);
    return (ptr);
}