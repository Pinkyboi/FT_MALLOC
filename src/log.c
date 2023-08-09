#include "malloc.h"

void show_alloc_mem()
{
    t_hdr_block *block_hdr;

    pthread_mutex_lock(&g_mutex);
    printf("TINY : %p\n", g_zones.tiny);
    if (g_zones.tiny)
    {
        block_hdr = GET_ZONE_FIRST_HEADER(g_zones.tiny);
        while (IS_VALID_ZONE_ADDR(g_zones.tiny, block_hdr))
        {
            printf("%p - %p : %d bytes\n", GET_MEMORY_BLOCK(block_hdr), GET_BLOCK_FOOTER(block_hdr), block_hdr->size);
            block_hdr = GET_NEXT_HEADER(block_hdr, block_hdr->size);
        }
    }
    printf("SMALL : %p\n", g_zones.small);
    if (g_zones.small)
    {
        block_hdr = GET_ZONE_FIRST_HEADER(g_zones.small);
        while (IS_VALID_ZONE_ADDR(g_zones.small, block_hdr))
        {
            printf("%p - %p : %d bytes\n", GET_MEMORY_BLOCK(block_hdr), GET_BLOCK_FOOTER(block_hdr), block_hdr->size);
            block_hdr = GET_NEXT_HEADER(block_hdr, block_hdr->size);
        }
    }
    printf("LARGE : %p\n", g_zones.large);
    for (t_zone *zone_head = g_zones.large; zone_head; zone_head = zone_head->next)
        printf("%p - %p : %zu bytes\n", GET_L_MEMORY_BLOCK(zone_head), zone_head + zone_head->size, zone_head->size);
    pthread_mutex_unlock(&g_mutex);
}

static void print_hex(unsigned char *ptr, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        if (i % 16 == 0)
            printf("%p : ", ptr + i);
        printf("%02x ", *(unsigned char *)(ptr + i));
        if (i % 16 == 15)
            printf("\n");
    }
    printf("\n");
}

void show_alloc_mem_ex()
{
    t_hdr_block *block_hdr;

    pthread_mutex_lock(&g_mutex);
    printf("TINY : %p\n", g_zones.tiny);
    block_hdr = GET_ZONE_FIRST_HEADER(g_zones.tiny);
    if (g_zones.tiny)
    {
        while (IS_VALID_ZONE_ADDR(g_zones.tiny, block_hdr))
        {
            printf("%p - %p : %d bytes\n", GET_MEMORY_BLOCK(block_hdr), GET_BLOCK_FOOTER(block_hdr), block_hdr->size);
            if (block_hdr->is_free == false)
                print_hex((unsigned char *)GET_MEMORY_BLOCK(block_hdr), GET_BLOCK_SIZE(block_hdr));
            block_hdr = GET_NEXT_HEADER(block_hdr, block_hdr->size);
        }
    }
    printf("SMALL : %p\n", g_zones.small);
    if (g_zones.small)
    {
        block_hdr = GET_ZONE_FIRST_HEADER(g_zones.small);
        while (IS_VALID_ZONE_ADDR(g_zones.small, block_hdr))
        {
            printf("%p - %p : %d bytes\n", GET_MEMORY_BLOCK(block_hdr), GET_BLOCK_FOOTER(block_hdr), block_hdr->size);
            print_hex((unsigned char *)GET_MEMORY_BLOCK(block_hdr), block_hdr->size);
            block_hdr = GET_NEXT_HEADER(block_hdr, block_hdr->size);
        }
    }
    printf("LARGE : %p\n", g_zones.large);
    for (t_zone *zone_head = g_zones.large; zone_head; zone_head = zone_head->next)
    {
        printf("%p - %p : %zu bytes\n", GET_L_MEMORY_BLOCK(zone_head), zone_head + zone_head->size, zone_head->size);
        print_hex((unsigned char *)GET_L_MEMORY_BLOCK(zone_head), zone_head->size - sizeof(t_zone));
    }
    pthread_mutex_unlock(&g_mutex);
}