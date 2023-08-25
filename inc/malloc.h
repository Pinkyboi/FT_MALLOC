
#ifndef _MALLOC_H
#define _MALLOC_H

#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/resource.h>
#include <stdint.h>
#include <pthread.h>

#define TINY_MAX 32
#define TINY_MIN 1
#define SMALL_MAX 1024
#define SMALL_MIN TINY_MAX + 1

#define IS_TINY(x) (x <= TINY_MAX)
#define IS_SMALL(x) (x > TINY_MAX && x <= SMALL_MAX)
#define IS_LARGE(x) (x > SMALL_MAX)

#define CEIL(x) ((x - (int)x > 0.0) ? (int)x + 1 : (int)x)

#define METADATA_SIZE (sizeof(t_hdr_block) + sizeof(t_ftr_block))
#define EFFECTIVE_SIZE(x) (x + METADATA_SIZE)
#define CEIL_DIV(a, b) ((a / b) + ((a % b) != 0))

#define SMALL_ZONE_SIZE (CEIL_DIV(((100 * EFFECTIVE_SIZE(SMALL_MIN)) + sizeof(t_zone) + sizeof(t_hdr_block)), getpagesize()) * getpagesize())
#define TINY_ZONE_SIZE  (CEIL_DIV(((100 * EFFECTIVE_SIZE(TINY_MIN)) + sizeof(t_zone) + sizeof(t_hdr_block)), getpagesize()) * getpagesize())
#define LARGE_ZONE_SIZE(x) ((CEIL_DIV((x + sizeof(t_zone)), getpagesize())) * getpagesize())

#define ZONE_SIZE(type) (type == TINY_ZONE ? TINY_ZONE_SIZE : SMALL_ZONE_SIZE)

#define FIRST_BLOCK_SIZE(type) (ZONE_SIZE(type) - sizeof(t_zone) - METADATA_SIZE)

#define GET_RIGHT_ZONE(size) (IS_TINY(size) ? g_zones.tiny : IS_SMALL(size) ? g_zones.small : g_zones.large)
#define GET_ZONE_BY_TYPE(type) (type == TINY_ZONE ? g_zones.tiny : type == SMALL_ZONE ? g_zones.small : g_zones.large)
#define GET_ZONE_TAIL(type) (type == TINY_ZONE ? g_zones.tiny_tail : type == SMALL_ZONE ? g_zones.small_tail : g_zones.large_tail)
#define GET_ZONE_TAIL_ADDR(type) (type == TINY_ZONE ? &g_zones.tiny_tail : type == SMALL_ZONE ? &g_zones.small_tail : &g_zones.large_tail)
#define GET_ZONE_ADDR(type) (type == TINY_ZONE ? &g_zones.tiny : type == SMALL_ZONE ? &g_zones.small : &g_zones.large)
#define GET_ZONE_NAME(type) (type == TINY_ZONE ? "TINY" : type == SMALL_ZONE ? "SMALL" : "LARGE")

#define GET_ZONE_FIRST_HEADER(zone) ((t_hdr_block *)((void *)zone + sizeof(t_zone) + sizeof(t_hdr_block)))

#define GET_BLOCK_HEADER(block) ((void *)block - sizeof(t_hdr_block))
#define GET_LBLOCK_HEADER(block) ((void *)block - sizeof(t_zone))
#define GET_MEMORY_BLOCK(hdr) ((void *)hdr + sizeof(t_hdr_block))
#define GET_L_MEMORY_BLOCK(zone) ((void *)zone + sizeof(t_zone))
#define GET_BLOCK_FOOTER(hdr) ((t_ftr_block *)((void *)hdr + hdr->size + sizeof(t_hdr_block) - sizeof(t_ftr_block)))
#define GET_NEXT_HEADER(hdr, size) ((t_hdr_block *)((void *)hdr + sizeof(t_hdr_block) + size))
#define GET_PREV_HEADER(hdr) ((t_hdr_block *)((void *)hdr - *((t_ftr_block *)((void *)hdr - sizeof(t_ftr_block))) - sizeof(t_hdr_block)))
#define GET_BLOCK_SIZE(hdr) (hdr->size - sizeof(t_ftr_block))
#define GET_L_BLOCK_SIZE(zone) (zone->size - sizeof(t_zone))


#define PRINT_ADDR(addr) write(1, "0x", 2); print_base((long long)addr, 16)
#define PUT_NBR(number) print_base(number, 10)

#define MIN(a, b) (a < b ? a : b)

#define IS_VALID_ZONE_ADDR(zone, addr) ((void *)zone < (void *)addr && (void *)addr < (void *)zone + zone->size)

#if __WORDSIZE == 32
    #define ALLIGN(x) (((((x) - 1) >> 2) << 2) + 4)
#else
    #define ALLIGN(x) (((((x) - 1) >> 3) << 3) + 8)
#endif



typedef enum        e_bool
{
    false = 0,
    true  = 1
}                   t_bool;

typedef enum        e_zone_type
{
    TINY_ZONE,
    SMALL_ZONE,
    LARGE_ZONE
}                   t_zone_type;

#if __WORDSIZE == 32
typedef uint16_t    t_ftr_block;
// strcutre representing a block of memory
typedef struct      s_hdr_block
{
    uint16_t        is_free: 1;    // is the block free
    uint16_t        size: 15;   // size of the block
}                   t_hdr_block;
#else
typedef uint32_t    t_ftr_block;

typedef struct      s_hdr_block
{
    uint32_t        is_free: 1;    // is the block free
    uint32_t        size: 31;   // size of the block
}                   t_hdr_block;
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
    t_zone          *tiny_tail;      // pointer to the next zone
    t_zone          *small;     // pointer to the small zone
    t_zone          *small_tail;      // pointer to the next zone
    t_zone          *large;     // pointer to the large zone
    t_zone          *large_tail;      // pointer to the next zone
}                   t_zones;

extern t_zones          g_zones;
extern pthread_mutex_t  g_mutex;

void            *ft_malloc(size_t size);
void            *malloc(size_t size);

void            *ft_realloc(void *ptr, size_t size);
void            *reallocf(void *ptr, size_t size);
void            *realloc(void *ptr, size_t size);

void            *calloc(size_t count, size_t size);

void            ft_free(void *ptr);
void            free(void *ptr);

void            show_alloc_mem();
void            show_alloc_mem_ex();

void            set_block_metadata(t_hdr_block *memory_block, t_bool is_free, size_t size);
size_t          get_alligned_size(size_t size);
t_hdr_block*    search_in_zone(void *ptr, t_zone_type zone_type);
t_zone*         search_in_large_zone(void *ptr);


void print_base(long long number, unsigned short base);
#endif