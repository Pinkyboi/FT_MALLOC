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
    t_zone *zone;
    t_zone **zone_tail;

    zone_tail = GET_ZONE_TAIL_ADDR(LARGE_ZONE);
    for (t_zone **zone_head = GET_ZONE_ADDR(LARGE_ZONE); *zone_head; zone_head = &(*zone_head)->next)
    {
        if((void *)GET_L_MEMORY_BLOCK(*zone_head) == ptr)
        {
            if (GET_ZONE_ADDR(LARGE_ZONE) == zone_head)
                break;
            zone = *zone_head;
            *zone_head =  (*zone_head)->next;
            if (*zone_tail == zone)
                *zone_tail = *zone_head;
            munmap(zone, zone->size);
            break;
        }
    }
}

void free_block(t_hdr_block *block_hdr, t_zone_type zone_type)
{
    t_zone *zone;
    t_zone **zone_tail;

    zone_tail = GET_ZONE_TAIL_ADDR(zone_type);
    block_hdr->is_free = true;
    merge_memory_blocks(block_hdr, GET_NEXT_HEADER(block_hdr, block_hdr->size));
    merge_memory_blocks(GET_PREV_HEADER(block_hdr), block_hdr);
    for (t_zone **zone_head = GET_ZONE_ADDR(zone_type); *zone_head; zone_head = &(*zone_head)->next)
    {
        if (FIRST_BLOCK_SIZE(zone_type) == GET_ZONE_FIRST_HEADER(*zone_head)->size)
        {
            if (GET_ZONE_ADDR(zone_type) == zone_head)
                break;
            zone = *zone_head;
            *zone_head =  (*zone_head)->next;
            if (*zone_tail == zone)
                *zone_tail = *zone_head;
            munmap(zone, zone->size);
            break;
        }
    }
}

void ft_free(void *ptr)
{
    t_hdr_block *block_hdr;

    if (ptr == NULL)
        return ;
    if ((block_hdr = search_in_zone(ptr, TINY_ZONE))) 
        free_block(block_hdr, TINY_ZONE);
    else if ((block_hdr = search_in_zone(ptr, SMALL_ZONE)))
        free_block(block_hdr, SMALL_ZONE);
    else
        free_large_block(ptr);
}

void free(void *ptr)
{
    pthread_mutex_lock(&g_mutex);
    ft_free(ptr);
    pthread_mutex_unlock(&g_mutex);
}