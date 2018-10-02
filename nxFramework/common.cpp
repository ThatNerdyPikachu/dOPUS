#include "common.h"

void printBytes(unsigned char *bytes, size_t size, bool includeHeader)
{
    int count = 0;

    if (includeHeader)
    {
        printf("\n\n00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
        printf("-----------------------------------------------\n");
    }

    for (int i = 0; i < (int)size; i++)
    {
        printf("%02x ", bytes[i]);
        count++;
        if ((count % 16) == 0)
            printf("\n");
    }
    printf("\n");
}
