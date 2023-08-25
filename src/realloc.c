#include "malloc.h"

static void *ft_memcpy(void *restrict dst, const void *restrict src, size_t n)
{
    for (size_t i = 0; i < n; i++)
        ((char *)dst)[i] = ((char *)src)[i];
    return (dst);
}

void    *ft_realloc(void *ptr, size_t size)
{
    void        *new_block;
    t_hdr_block *old_header;
    t_zone      *zone;
    size_t      leftover;

    old_header = NULL;
    new_block  = NULL;
    zone       = NULL;
    if (ptr == NULL)
        return ft_malloc(size);
    if ((old_header = search_in_zone(ptr, TINY_ZONE)) || (old_header = search_in_zone(ptr, SMALL_ZONE)))
    {
        size = get_alligned_size(size);
        if (size == old_header->size)
            return (ptr);
        leftover = old_header->size - size;
        if (size <= old_header->size && leftover > (ssize_t)METADATA_SIZE)
        {
            set_block_metadata(GET_NEXT_HEADER(old_header, size), true, leftover - sizeof(t_hdr_block));
            set_block_metadata(old_header, false, size);
            return ptr;
        }
    }
    if (old_header == NULL && (zone = search_in_large_zone(ptr)) == NULL)
        return NULL;
    if (!(new_block = ft_malloc(size)))
        return NULL;
    if (zone)
        ft_memcpy(new_block, ptr, MIN(size, GET_L_BLOCK_SIZE(zone)));
    else
        ft_memcpy(new_block, ptr, MIN(size, GET_BLOCK_SIZE(old_header)));
    ft_free(ptr);
    return new_block;
}

void    *realloc(void *ptr, size_t size)
{
    void    *new_ptr;

    pthread_mutex_lock(&g_mutex);
    new_ptr = ft_realloc(ptr, size);
    pthread_mutex_unlock(&g_mutex);
    return new_ptr;
}