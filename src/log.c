#include "malloc.h"

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