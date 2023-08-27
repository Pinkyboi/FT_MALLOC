#include "malloc.h"

inline size_t  get_alligned_size(size_t size)
{
    return (ALLIGN(size) + sizeof(t_ftr_block));
}

void set_block_metadata(t_hdr_block *memory_block, t_bool is_free, size_t size)
{
    *memory_block = (t_hdr_block){.is_free = is_free, .size = size};
    *GET_BLOCK_FOOTER(memory_block) = size;
}

t_hdr_block *search_in_zone(void *ptr, t_zone_type zone_type)
{
    t_hdr_block *curr_block;
    t_zone      *zone;

    zone = GET_ZONE_BY_TYPE(zone_type);
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

t_bool is_allocated(void *ptr)
{
    t_hdr_block *block;

    if ((block = search_in_zone(ptr, TINY_ZONE)) || (block = search_in_zone(ptr, SMALL_ZONE)))
        return (block->is_free == false);
    if (search_in_large_zone(ptr))
        return (true);
    return (false);
}