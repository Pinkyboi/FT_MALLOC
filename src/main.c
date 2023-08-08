#include "malloc.h"

t_zones g_zones = (t_zones){0};
pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;

inline size_t  get_alligned_size(size_t size)
{
    size_t alligned_size;

    alligned_size = ALLIGN(size);
    return (alligned_size - size >= sizeof(t_ftr_block) ? alligned_size : alligned_size + sizeof(t_ftr_block) );
}

void set_block_metadata(t_hdr_block *memory_block, t_bool is_free, size_t size)
{
    *memory_block = (t_hdr_block){is_free, size};
    *GET_BLOCK_FOOTER(memory_block) = size;
}

void *alloc_block(t_zone *zone, size_t size)
{
    t_hdr_block *curr_block_hdr;
    t_hdr_block *best_block_hdr;
    size_t      leftover_size;

    best_block_hdr = NULL;
    for (t_zone *zone_head = zone; zone_head; zone_head = zone_head->next)
    {
        curr_block_hdr = GET_ZONE_FIRST_HEADER(zone_head);
        while ((void*)curr_block_hdr - (void*)zone_head < zone->size)
        {
            if (curr_block_hdr->size + (void *)curr_block_hdr - (void *)zone_head > zone->size)
                break;
            if (curr_block_hdr->is_free && size <= curr_block_hdr->size && (!best_block_hdr || curr_block_hdr->size < best_block_hdr->size))
                best_block_hdr = curr_block_hdr;
            curr_block_hdr = GET_NEXT_HEADER(curr_block_hdr, curr_block_hdr->size);
        }
    }
    if (best_block_hdr)
    {
        leftover_size = best_block_hdr->size - size;
        if (leftover_size > METADATA_SIZE)
            set_block_metadata(GET_NEXT_HEADER(best_block_hdr, size), true, leftover_size - sizeof(t_hdr_block));
        set_block_metadata(best_block_hdr, false, size);
        return (GET_MEMORY_BLOCK(best_block_hdr));
    }
    ft_zone_init(size);
    return (alloc_block(zone, size));
}

t_bool alloc_memory_page(t_zone **zone, t_zone **zone_tail, size_t size)
{
    t_zone *mapped_memory;

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
        *zone_tail = mapped_memory;
    }
    return (true);
}

t_bool ft_zone_init(size_t size)
{
    if ((IS_TINY(size) && g_zones.tiny != NULL) || (IS_SMALL(size) && g_zones.small != NULL))
        return (true);
    if (IS_TINY(size))
    {
        if (alloc_memory_page(&g_zones.tiny, &g_zones.tiny_tail, TINY_ZONE_SIZE) == false)
            return (false);
    }
    if (IS_SMALL(size))
    {
        if (alloc_memory_page(&g_zones.small, &g_zones.small_tail, SMALL_ZONE_SIZE) == false)
            return (false);
    }
    set_block_metadata(GET_ZONE_FIRST_HEADER(GET_RIGHT_TAIL(size)), true, FIRST_BLOCK_SIZE(size));
    return ((IS_TINY(size) && g_zones.tiny != NULL) || (IS_SMALL(size) && g_zones.small != NULL));
}

t_hdr_block *search_in_zone(void *ptr, t_zone_type zone_type)
{
    t_hdr_block *curr_block;
    t_zone      *zone;
    size_t      zone_size;

    zone = (zone_type == TINY_ZONE) ? g_zones.tiny : g_zones.small;
    zone_size = (zone_type == TINY_ZONE) ? TINY_ZONE_SIZE : SMALL_ZONE_SIZE;
    for (t_zone *zone_head = zone; zone_head; zone_head = zone_head->next)
    {
        curr_block = GET_ZONE_FIRST_HEADER(zone_head);
        while (IS_VALID_ZONE_ADDR(zone_head, curr_block))
        {
            if(GET_MEMORY_BLOCK(curr_block) == ptr)
                return (curr_block);
            curr_block = GET_NEXT_HEADER(curr_block, curr_block->size);
        }
    }
    return (NULL);
}

t_zone *search_in_large_zone(void *ptr)
{
    for (t_zone *zone_head = g_zones.large; zone_head; zone_head = zone_head->next)
        if((void *)GET_L_MEMORY_BLOCK(zone_head) == ptr)
            return (zone_head);
    return (NULL);
}

void merge_memory_blocks(t_hdr_block *first_block, t_hdr_block *second_block)
{
    if (first_block->is_free == false || second_block->is_free == false)
        return ;
    first_block->size += second_block->size + sizeof(t_hdr_block);
    *GET_BLOCK_FOOTER(first_block) = first_block->size;
}

void free_large_block(void *ptr)
{
    t_zone *next_head;
    t_zone *zone_head;

    if ((zone_head = search_in_large_zone(ptr)) == NULL)
        return ;
    next_head = zone_head->next;
    munmap(zone_head, zone_head->size);
    zone_head = next_head;
}
void ft_free(void *ptr)
{
    t_hdr_block *block_hdr;

    if ((block_hdr = search_in_zone(ptr, TINY_ZONE)) || (block_hdr = search_in_zone(ptr, SMALL_ZONE)))
    {
        block_hdr->is_free = true;
        merge_memory_blocks(block_hdr, GET_NEXT_HEADER(block_hdr, block_hdr->size));
        merge_memory_blocks(GET_PREV_HEADER(block_hdr), block_hdr);
    }
    else
        free_large_block(ptr);
}

void *ft_malloc(size_t size)
{
    void *ptr;

    if (size == 0)
        return (NULL);
    if (IS_LARGE(size))
    {
        if (alloc_memory_page(&g_zones.large, &g_zones.large_tail, LARGE_ZONE_SIZE(size)))
            return (GET_L_MEMORY_BLOCK(g_zones.large_tail));
        return (NULL);
    }
    size = get_alligned_size(size);
    if (ZONES_NOT_ALLOCATED(g_zones) && ft_zone_init(size) == false)
        return (NULL);
    ptr = alloc_block(GET_RIGHT_ZONE(size), size);
    return (ptr);
}

void log_zone(t_zone *zone)
{
    t_hdr_block *block_hdr;

    for (t_zone *zone_head = zone; zone_head; zone_head = zone_head->next)
    {
        printf("Zone: %p, size: %zu\n", zone_head, zone_head->size);
        block_hdr = GET_ZONE_FIRST_HEADER(zone_head);
        while (IS_VALID_ZONE_ADDR(zone_head, block_hdr))
        {
            printf("Block: %p, size: %d, is_free: %d\n", block_hdr, block_hdr->size, block_hdr->is_free);
            block_hdr = GET_NEXT_HEADER(block_hdr, block_hdr->size);
        }
    }
}

void *ft_memcpy(void *restrict dst, const void *restrict src, size_t n)
{
    for (size_t i = 0; i < n; i++)
        ((char *)dst)[i] = ((char *)src)[i];
    return (dst);
}

void *ft_memset(void *b, int c, size_t len)
{
    for (size_t i = 0; i < len; i++)
        ((char *)b)[i] = c;
    return (b);
}

void    *ft_realloc(void *ptr, size_t size)
{
    t_hdr_block *old_header;
    t_zone      *zone;
    void        *new_block;
    ssize_t      leftover;

    old_header = NULL;
    new_block  = NULL;
    zone       = NULL;
    if ((old_header = search_in_zone(ptr, TINY_ZONE)) || (old_header = search_in_zone(ptr, SMALL_ZONE)))
    {
        size = get_alligned_size(size);
        if (size == old_header->size)
            return (ptr);
        leftover = (ssize_t)(old_header->size) - size;
        if (leftover > (ssize_t)METADATA_SIZE)
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

void *ft_calloc(size_t count, size_t size)
{
    void *ptr;

    if ((ptr = ft_malloc(count * size)) == NULL)
        return (NULL);
    ft_memset(ptr, 0, count * size);
    return (ptr);
}
void show_alloc_mem()
{
    t_hdr_block *block_hdr;

    pthread_mutex_lock(&g_mutex);
    printf("TINY : %p\n", g_zones.tiny);
    block_hdr = GET_ZONE_FIRST_HEADER(g_zones.tiny);
    while (IS_VALID_ZONE_ADDR(g_zones.tiny, block_hdr))
    {
        printf("%p - %p : %d bytes\n", GET_MEMORY_BLOCK(block_hdr), GET_BLOCK_FOOTER(block_hdr), block_hdr->size);
        block_hdr = GET_NEXT_HEADER(block_hdr, block_hdr->size);
    }
    printf("SMALL : %p\n", g_zones.small);
    block_hdr = GET_ZONE_FIRST_HEADER(g_zones.small);
    while (IS_VALID_ZONE_ADDR(g_zones.small, block_hdr))
    {
        printf("%p - %p : %d bytes\n", GET_MEMORY_BLOCK(block_hdr), GET_BLOCK_FOOTER(block_hdr), block_hdr->size);
        block_hdr = GET_NEXT_HEADER(block_hdr, block_hdr->size);
    }
    printf("LARGE : %p\n", g_zones.large);
    for (t_zone *zone_head = g_zones.large; zone_head; zone_head = zone_head->next)
        printf("%p - %p : %zu bytes\n", GET_L_MEMORY_BLOCK(zone_head), zone_head + zone_head->size, zone_head->size);
    pthread_mutex_unlock(&g_mutex);
}
int main()
{
    // printf("%zu\n", sizeof(t_zone));
    // char *nejma = ft_malloc(10);
    // char *nejma2 = ft_malloc(10);
    // char *nejma3 = ft_malloc(1000000);
    // memset(nejma3, 'a', 30);
    char *nejma4 = ft_malloc(50);

    // printf("%p\n", nejma4);
    // ft_free(nejma3);
    nejma4 = ft_realloc(nejma4, 300);
    nejma4 = ft_malloc(10000);
    // nejma4 = ft_realloc(nejma4, 50);
    // nejma4 = ft_realloc(nejma4, 1000);
    // char *nejma3 = ft_malloc(40);
    // char *nejma4 = ft_malloc(40);
    // char *nejma5 = ft_malloc(3000);
    // log_zone(g_zones.small);
    show_alloc_mem();
    // printf("%p", nejma4);
    // ft_free(nejma);
    // printf("------------------\n");
    // log_zone(g_zones.tiny);
    // ft_free(nejma4);
    // ft_free(nejma3);
    // nejma = ft_malloc(2);
    // printf("------------------\n");
    // log_zone(g_zones.tiny);
    // ft_free(nejma2);
    // ft_free(nejma);
    // printf("------------------\n");
    // log_zone(g_zones.tiny);


    // *nejma5 = 'a';
    // *(nejma5 + 1) = '\0';
    // printf("%s\n", nejma5);
    // ft_free(nejma5);
    
    // printf("%p\n", nejma2);
    // printf("%p\n", nejma3);
    // printf("%p\n", nejma4);
    // printf("%p\n", nejma5);


    // printf("%p, %d\n", GET_ZONE_FIRST_HEADER(g_zones.tiny), GET_ZONE_FIRST_HEADER(g_zones.tiny)->size);
    // printf("%c\n", *nejma);
    // printf("%p, %d\n", GET_BLOCK_FOOTER(GET_ZONE_FIRST_HEADER(g_zones.tiny)),  *GET_BLOCK_FOOTER(GET_ZONE_FIRST_HEADER(g_zones.tiny)));
}