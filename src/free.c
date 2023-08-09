#include "malloc.h"

static void merge_memory_blocks(t_hdr_block *first_block, t_hdr_block *second_block)
{
    if (first_block->is_free == false || second_block->is_free == false)
        return ;
    first_block->size += second_block->size + sizeof(t_hdr_block);
    *GET_BLOCK_FOOTER(first_block) = first_block->size;
}

static void free_large_block(void *ptr)
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

    if (ptr == NULL)
        return ;
    if ((block_hdr = search_in_zone(ptr, TINY_ZONE)) || (block_hdr = search_in_zone(ptr, SMALL_ZONE)))
    {
        block_hdr->is_free = true;
        merge_memory_blocks(block_hdr, GET_NEXT_HEADER(block_hdr, block_hdr->size));
        merge_memory_blocks(GET_PREV_HEADER(block_hdr), block_hdr);
    }
    else
        free_large_block(ptr);
}

void free(void *ptr)
{
    pthread_mutex_lock(&g_mutex);
    ft_free(ptr);
    pthread_mutex_unlock(&g_mutex);
}