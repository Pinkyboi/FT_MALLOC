
#ifndef _MALLOC_H
#define _MALLOC_H

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include <pthread.h>
#include <math.h>

#define TINY_SIZE 128
#define SMALL_SIZE 1024

#define IS_TINY(x) (x <= TINY_SIZE)
#define IS_SMALL(x) (x > TINY_SIZE && x <= SMALL_SIZE)
#define IS_LARGE(x) (x > SMALL_SIZE)

#define EFFECTIVE_SIZE(x) (x + sizeof(t_hdr_block) + sizeof(t_ftr_block))
#define LARGE_BLOCK_SIZE(x) (x + sizeof(t_zone))

#define METADATA_SIZE (sizeof(t_hdr_block) + sizeof(t_ftr_block))
#define TINY_ZONE_SIZE  ceil(((100 * (TINY_SIZE + METADATA_SIZE)) + sizeof(t_zone) + sizeof(t_hdr_block)) / (double)getpagesize()) * getpagesize()
#define SMALL_ZONE_SIZE ceil(((100 * (SMALL_SIZE + METADATA_SIZE)) + sizeof(t_zone) + sizeof(t_hdr_block) / (double)getpagesize())) * getpagesize()

#define ZONE_SIZE(size) (IS_TINY(size) ? TINY_ZONE_SIZE : SMALL_ZONE_SIZE)
#define FIRST_BLOCK_SIZE(size) (ZONE_SIZE(size) - sizeof(t_zone) - METADATA_SIZE)

#define GET_RIGHT_ZONE(size) (IS_TINY(size) ? g_zones.tiny : IS_SMALL(g_zones.small) ? g_zones.small : g_zones.large)
// #define GET_RIGHT_ZONE(size, zone_type) (zone_type == TINY_ZONE ? g_zones.tiny : zone_type == SMALL_ZONE ? g_zones.small : g_zones.large)

#define GET_RIGHT_TAIL(size) (IS_TINY(size) ? g_zones.tiny_tail : g_zones.small_tail)

#define ZONES_NOT_ALLOCATED(zones) (!zones.tiny && !zones.small)

#define GET_ZONE_FIRST_HEADER(zone) ((t_hdr_block *)((void *)zone + sizeof(t_zone) + sizeof(t_hdr_block)))

#define GET_BLOCK_HEADER(block) ((void *)block - sizeof(t_hdr_block))
#define GET_MEMORY_BLOCK(hdr) ((void *)hdr + sizeof(t_hdr_block))
#define GET_L_MEMORY_BLOCK(zone) ((void *)zone + sizeof(t_zone))
#define GET_BLOCK_FOOTER(hdr) ((t_ftr_block *)((void *)hdr + hdr->size + sizeof(t_hdr_block) - sizeof(t_ftr_block)))
#define GET_NEXT_HEADER(hdr, size) ((t_hdr_block *)((void *)hdr + sizeof(t_hdr_block) + size))
#define GET_PREV_HEADER(hdr) ((t_hdr_block *)((void *)hdr - *((t_ftr_block *)((void *)hdr - sizeof(t_ftr_block))) - sizeof(t_hdr_block)))

#define IS_VALID_ZONE_ADDR(zone, addr) ((void *)zone < (void *)addr && (void *)addr < (void *)zone + zone->size)

#if __WORDSIZE == 32
    #define ALLIGN(x) (((((x) - 1) >> 2) << 2) + 4)
#else
    #define ALLIGN(x) (((((x) - 1) >> 3) << 3) + 8)
#endif



typedef enum e_bool
{
    false = 0,
    true  = 1
}           t_bool;

typedef enum e_zone_type
{
    TINY_ZONE,
    SMALL_ZONE,
    LARGE_ZONE
}           t_zone_type;

#if __WORDSIZE == 32
    typedef uint16_t  t_ftr_block;
    // strcutre representing a block of memory
    typedef struct          s_hdr_block
    {
        uint16_t  is_free: 1;    // is the block free
        uint16_t  size: 15;   // size of the block
    }                       t_hdr_block;
#else
    typedef uint32_t  t_ftr_block;
    // strcutre representing a block of memory
    typedef struct          s_hdr_block
    {
        uint32_t  is_free: 1;    // is the block free
        uint32_t  size: 31;   // size of the block
    }                       t_hdr_block;
#endif

// structure representing a memory zone (tiny, small or large)
typedef struct      s_zone
{
    size_t          size;     // pointer to the end of the zone
    struct s_zone   *next;      // pointer to the next zone
}                   t_zone;

// structure containing the different zones
typedef struct      s_zones
{
    t_zone          *tiny;      // pointer to the tiny zone
    struct s_zone   *tiny_tail;      // pointer to the next zone
    t_zone          *small;     // pointer to the small zone
    struct s_zone   *small_tail;      // pointer to the next zone
    t_zone          *large;     // pointer to the large zone
    struct s_zone   *large_tail;      // pointer to the next zone
}                   t_zones;

extern t_zones      g_zones;

void    *ft_malloc(size_t size);
void    *ft_realloc(void *ptr, size_t size);
void    ft_free(void *ptr);

size_t  get_alligned_size(size_t size);
t_bool  ft_zone_init(size_t size);
#endif