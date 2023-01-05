#include "framework/userspace.h"
#include <cstdio>
#include <cinttypes>
#include <arpa/inet.h>

struct IPv4Frame
{
    uint8_t ip_hl_v; 
    uint8_t ip_tos;
    unsigned short ip_len;
    unsigned short ip_id;
    unsigned short ip_off;
    uint8_t ip_ttl;
    uint8_t ip_p;
    unsigned short ip_sum;
    uint8_t ip_src[4], ip_dst[4];
    uint8_t options_payload[];
}__attribute__((__packed__));

int counter_out = 0;
int counter_in = 0;
int counter_tcp = 0;

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

    auto &ip = *reinterpret_cast<const IPv4Frame*>(packetData);
    bool port443 = false;
    bool port80 = false;
    bool private_addr_out = false;
    bool private_addr_in = false;
    
    if((ip.ip_src[0] == (uint8_t)8 && ip.ip_src[1] == (uint8_t)8 && ip.ip_src[2] == (uint8_t)4 && ip.ip_src[3] == (uint8_t)4)
    ||(ip.ip_dst[0] == (uint8_t)8 && ip.ip_dst[1] == (uint8_t)8 && ip.ip_dst[2] == (uint8_t)4 && ip.ip_dst[3] == (uint8_t)4))
    {
        response = DROP;
    }

    if(ip.ip_src[0] == (uint8_t)127 && ip.ip_dst[0] == (uint8_t)127)
    {
        response = ACCEPT;
    }
    
    if((ip.ip_off != 0))
    {
        if((ip.ip_off == 1)&&!((ip.ip_off>>14) == (unsigned short)1))
        {
            response = DROP;
        }

        if(ip.ip_off > 1)
        {
            response = DROP;
        }

    }
    
    if(ip.ip_p !=1  && ip.ip_p != 6 && ip.ip_p != 17)
    {
        response = DROP;
    }

    
    if((ip.ip_dst[0] == (uint8_t)10) ||( ip.ip_dst[0] == (uint8_t)192 && (ip.ip_dst[1]== (uint8_t)168)) || ((ip.ip_dst[0] == (uint8_t)172 && ((ip.ip_dst[1]>= (uint8_t)16 && ip.ip_dst[1] <= (uint8_t)31) ))))
    {
        private_addr_out = true;
    }

    if((ip.ip_src[0] == (uint8_t)10) ||( ip.ip_src[0] == (uint8_t)192 && (ip.ip_src[1]== (uint8_t)168)) || ((ip.ip_src[0] == (uint8_t)172 && ((ip.ip_src[1]>= (uint8_t)16 && ip.ip_src[1] <= (uint8_t)31) ))))
    {
        private_addr_in = true;
    }

    if((ip.ip_p == 1) && (!((ip.ip_src[0] == (uint8_t)8 && ip.ip_src[1] == (uint8_t)8 && ip.ip_src[2] == (uint8_t)4 && ip.ip_src[3] == (uint8_t)4)
    ||(ip.ip_dst[0] == (uint8_t)8 && ip.ip_dst[1] == (uint8_t)8 && ip.ip_dst[2] == (uint8_t)4 && ip.ip_dst[3] == (uint8_t)4))))
    {
        response = ACCEPT;

        if((ip.options_payload[0] == (uint8_t)8) && direction == OUTGOING && private_addr_out)
        {
            counter_out ++;
            if (counter_out % 3 == 0)
            {
                response = DROP;
            }
        }
     
        if((ip.options_payload[0] == (uint8_t)8) && direction == INCOMING && private_addr_in)
        {
            counter_in ++;
            if (counter_in % 4 == 0)
            {
                response = DROP;

            }
        }
    
    }

    if((ip.ip_p == 6) && (!((ip.ip_src[0] == (uint8_t)8 && ip.ip_src[1] == (uint8_t)8 && ip.ip_src[2] == (uint8_t)4 && ip.ip_src[3] == (uint8_t)4)
    ||(ip.ip_dst[0] == (uint8_t)8 && ip.ip_dst[1] == (uint8_t)8 && ip.ip_dst[2] == (uint8_t)4 && ip.ip_dst[3] == (uint8_t)4))))
    {
        
        if(((ip.options_payload[2]<<8) | (ip.options_payload[3])) == (uint8_t)433)
        {
            port443 = true;
        }
        
        if(((ip.options_payload[2]<<8) | (ip.options_payload[3])) == (uint8_t)80)
        {
            port80 = true;
        }
        
        if(direction==OUTGOING && port443)
        {
            response = DROP;
              
        }

        if(direction == OUTGOING && port80)
        {
            counter_tcp++;
            if(counter_tcp % 2 ==0)
            {
                response = DROP;
                
            }else
            {
                response= ACCEPT;
            }
        
        }

    }

    return response;
}
