#include "malloc.h"

inline static void putstr(const char *str)
{
    for (int i = 0; str[i]; i++)
        write(1, &str[i], 1);
}

void print_base(long long number, unsigned short base)
{
    const char  tokens[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char        base_number[sizeof(number) * 8];
    char        *base_pointer;               

    if (base > sizeof(tokens))
        return;
    base_pointer = base_number;
    if (number == 0)
        *base_pointer++ = '0';
    while (number)
    {
        *base_pointer++ = tokens[number % base];
        number /= base;
    }
    while (base_pointer-- != base_number - 1)
        write(1, base_pointer, 1);
}

static void print_hex(unsigned char *ptr, size_t size)
{
    for (size_t i = 0; i < size; i++)
    {
        if (i % 16 == 0)
            printf("%p: ", (void *)(ptr + i));
        if (i % 16 && i % 8 == 0)
            printf(" ");
        if (ptr[i])
            printf("%02x ", ptr[i]);
        else
            printf("\033[37m%02x\033[0m ", ptr[i]);
        if (i % 16 == 15 || i + 1 == size)
            printf("\n");
    }
}

inline static void print_mem_info(void *start, void *end, long size)
{
    PRINT_ADDR(start);
    putstr(" - ");
    PRINT_ADDR(end);
    putstr(" : \033[0;32m");
    PUT_NBR(size);
    putstr(" \033[0mbytes\n");
}

static void print_zone_infos(t_zone_type zone_type, t_bool extra_infos)
{
    t_zone      *chosen_zone;
    t_hdr_block *block_hdr;

    chosen_zone = GET_ZONE_BY_TYPE(zone_type);
    putstr(GET_ZONE_NAME(zone_type));
    putstr("\t: ");
    PRINT_ADDR(chosen_zone);
    putstr("\n");
    if (!chosen_zone)
        return ;
    if (zone_type == LARGE_ZONE)
    {
        for (t_zone *zone_head = chosen_zone; zone_head; zone_head = zone_head->next)
        {
            print_mem_info(GET_L_MEMORY_BLOCK(zone_head), zone_head + zone_head->size, zone_head->size);
            if (extra_infos)
                print_hex((unsigned char *)GET_L_MEMORY_BLOCK(zone_head), zone_head->size - sizeof(t_zone));
        }
    }
    else
    {
        block_hdr = GET_ZONE_FIRST_HEADER(chosen_zone);
        while (IS_VALID_ZONE_ADDR(chosen_zone, block_hdr))
        {
            print_mem_info(GET_MEMORY_BLOCK(block_hdr), GET_BLOCK_FOOTER(block_hdr), block_hdr->size);
            if (block_hdr->is_free == false && extra_infos)
                print_hex((unsigned char *)GET_MEMORY_BLOCK(block_hdr), block_hdr->size);
            block_hdr = GET_NEXT_HEADER(block_hdr, block_hdr->size);
        }
    }
}

void show_alloc_mem()
{
    pthread_mutex_lock(&g_mutex);
    print_zone_infos(TINY_ZONE, false);
    print_zone_infos(SMALL_ZONE, false);
    print_zone_infos(LARGE_ZONE, false);
    pthread_mutex_unlock(&g_mutex);
}

void show_alloc_mem_ex()
{
    pthread_mutex_lock(&g_mutex);
    print_zone_infos(TINY_ZONE, true);
    print_zone_infos(SMALL_ZONE, true);
    print_zone_infos(LARGE_ZONE, true);
    pthread_mutex_unlock(&g_mutex);
}