#include "framework/userspace.h"
#include <cstdio>

static void hexdump(uint8_t const* data, size_t size)
{
    for (size_t bi=0; bi<size; bi += 16)
    {
        // hex values
        for (size_t di=0; di<16; ++di)
        {
            size_t i = bi+di;
            if (i < size)
                printf("%02X ", data[i]);
            else
                printf("   ");
        }
        printf("        ");
        
        // printable characters
        for (size_t di=0; di<16; ++di)
        {
            size_t i = bi+di;
            if (i < size)
            {
                uint8_t c = data[i];
                if ((32 <= c) && (c <= 126))
                    printf(" %c ", c);
                else
                    printf(" . ");
            }
            else
                printf("  ");
        }
        printf("\n");
    }
}


FilterResponse processPacket(uint8_t const* packetData, size_t packetSize, Direction direction)
{
    struct _PNAP { ~_PNAP() { printf("\n"); } } printNewlineAfterPacket;
    
    switch (direction)
    {
        case INCOMING: printf("Packet (size %zu) received:\n", packetSize); break;
        case OUTGOING: printf("Packet (size %zu) sent:\n", packetSize); break;
    }
    
    hexdump(packetData, packetSize);

    FilterResponse response = ACCEPT;

// TODO: Implement this method!
    
    return response;
}
