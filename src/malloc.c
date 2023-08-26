#include "malloc.h"

t_zones g_zones = (t_zones){0};
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

static t_bool alloc_memory_page(t_zone **zone, t_zone **zone_tail, size_t size)
{
    t_zone *mapped_memory;
    t_zone *zone_tail_ptr;

    mapped_memory = (t_zone *)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
    if (mapped_memory == MAP_FAILED)
        return (false);
    mapped_memory->size = size;
    mapped_memory->next = NULL;
    if (*zone == NULL)
    {
        *zone = mapped_memory;
        *zone_tail = mapped_memory;
    }
    else
    {
        (*zone_tail)->next = mapped_memory;
        zone_tail_ptr = *zone_tail;
        *zone_tail = mapped_memory;
        (*zone_tail)->prev = zone_tail_ptr;
    }
    return (true);
}

static t_bool ft_zone_init(size_t size)
{
    if (IS_TINY(size))
    {
        if (alloc_memory_page(&g_zones.tiny, &g_zones.tiny_tail, TINY_ZONE_SIZE) == false)
            return (false);
        set_block_metadata(GET_ZONE_FIRST_HEADER(GET_ZONE_TAIL(TINY_ZONE)), true, FIRST_BLOCK_SIZE(TINY_ZONE));
    }
    if (IS_SMALL(size))
    {
        if (alloc_memory_page(&g_zones.small, &g_zones.small_tail, SMALL_ZONE_SIZE) == false)
            return (false);
        set_block_metadata(GET_ZONE_FIRST_HEADER(GET_ZONE_TAIL(SMALL_ZONE)), true, FIRST_BLOCK_SIZE(SMALL_ZONE));
    }
    return (true);
}

static void *alloc_block(t_zone *zone, size_t size)
{
    t_hdr_block *curr_block_hdr;
    t_hdr_block *best_block_hdr;
    size_t      aligned_size;
    size_t      leftover_size;

    best_block_hdr = NULL;
    aligned_size = get_alligned_size(size);
    for (t_zone *zone_head = zone; zone_head; zone_head = zone_head->next)
    {
        curr_block_hdr = GET_ZONE_FIRST_HEADER(zone_head);
        while (IS_VALID_ZONE_ADDR(zone_head, curr_block_hdr))
        {
            if (curr_block_hdr->is_free && aligned_size <= curr_block_hdr->size &&\
                (!best_block_hdr || curr_block_hdr->size < best_block_hdr->size))
                best_block_hdr = curr_block_hdr;
            curr_block_hdr = GET_NEXT_HEADER(curr_block_hdr, curr_block_hdr->size);
        }
    }
    if (best_block_hdr)
    {
        leftover_size = best_block_hdr->size - aligned_size;
        if (leftover_size > METADATA_SIZE)
            set_block_metadata(GET_NEXT_HEADER(best_block_hdr, aligned_size), true, leftover_size - sizeof(t_hdr_block));
        set_block_metadata(best_block_hdr, false, aligned_size);
        return (GET_MEMORY_BLOCK(best_block_hdr));
    }
    ft_zone_init(size);
    return (alloc_block(zone, size));
}

static inline void *get_large_alloc(size_t size)
{
    struct rlimit   zone_size_limit;

    getrlimit(RLIMIT_DATA, &zone_size_limit);
    if (LARGE_ZONE_SIZE(size) > zone_size_limit.rlim_cur)
        return (NULL);
    if (alloc_memory_page(&g_zones.large, &g_zones.large_tail, LARGE_ZONE_SIZE(size)))
        return (GET_L_MEMORY_BLOCK(g_zones.large_tail));
    return (NULL);
}

void *ft_malloc(size_t size)
{
    if (size == 0)
        return (NULL);
    if (IS_LARGE(size))
        return (get_large_alloc(size));
    if (!GET_RIGHT_ZONE(size) && ft_zone_init(size) == false)
        return (NULL);
    return (alloc_block(GET_RIGHT_ZONE(size), size));
}

void *malloc(size_t size)
{
    void *ptr;
    pthread_mutex_lock(&g_mutex);
    ptr = ft_malloc(size);
    pthread_mutex_unlock(&g_mutex);
    return (ptr);
}
