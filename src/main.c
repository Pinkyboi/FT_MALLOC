#include "malloc.h"

int main()
{
    char *nejma4 = ft_malloc(50);
    nejma4 = ft_realloc(nejma4, 300);
    nejma4 = ft_malloc(10000);
    show_alloc_mem();
}