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
}__attributte__;

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
    int counter_out = 0;
    int counter_in = 0;
    bool type = false;
    bool port443 = false;
    bool port80 = false;
    int counter_tcp = 0;
    //bool address_dest = false;
    //bool addres_src = false;
    
    if((ip.ip_src[0] == (uint8_t)8 && ip.ip_src[1] == (uint8_t)8 && ip.ip_src[2] == (uint8_t)4 && ip.ip_src[3] == (uint8_t)4)
    ||(ip.ip_dst[0] == (uint8_t)8 && ip.ip_dst[1] == (uint8_t)8 && ip.ip_dst[2] == (uint8_t)4 && ip.ip_dst[3] == (uint8_t)4))
    {
        response = DROP;
        return response;
    }

    if(ip.ip_src[0] == (uint8_t)127 && ip.ip_dst[0] == (uint8_t)127)
    {
        response = ACCEPT;
    }
    
    if(ip.ip_off > 0)
    {
        response = DROP;
    }
    
    if(ip.ip_p !=1  && ip.ip_p != 6 && ip.ip_p != 17)
    {
        response = DROP;
    }

    for(int i = 0; i < 4; i++)
    {
        if(!ip.options_payload[i])
        {
            type = true;
        }
        else
        {
            type = false;
        }

        if(type == false)
        {
            break;
        }

    }

    if(type == true)
    {
        for(int i = 4; i < 8; i++)
        {
            if(ip.options_payload[4])
            {
                type = true;
                if(!ip.options_payload[i])
                {
                    type = true;
                }
                else{
                    type = false;
                    break;
                }
            }
            else
            {
                type = false;
                break;
            }

        }
    }



    if(ip.ip_p == 1)
    {
        if(type && direction == OUTGOING)
        {
            counter_out ++;
            if (counter_out == 3)
            {
                counter_out = 0;
                response = DROP;
                return response;
            } 
        }
        else{

             response = ACCEPT;
 

        }        

        if(type && direction == INCOMING)
        {
            counter_in ++;
            if (counter_in == 4)
            {
                counter_in = 0;
                response = DROP;
               
            } 
        }
        else{

             response = ACCEPT;

        }
    
    }

    if(ip.ip_p == 6)
    {
        
        if(!ip.options_payload[16] && !ip.options_payload[17] && !ip.options_payload[18] && !ip.options_payload[19] && !ip.options_payload[20] && !ip.options_payload[21]&& !ip.options_payload[22]
        && ip.options_payload[23] && ip.options_payload[24] && !ip.options_payload[25] && ip.options_payload[26] && ip.options_payload[27] && ip.options_payload[28] 
        && !ip.options_payload[29] && ip.options_payload[31] && ip.options_payload[32])
        {
            port443 = true;
        }
        
        if(!ip.options_payload[16] && !ip.options_payload[17] && !ip.options_payload[18] && !ip.options_payload[19] && !ip.options_payload[20] && !ip.options_payload[21]&& !ip.options_payload[22]
        && !ip.options_payload[23] && !ip.options_payload[24] && ip.options_payload[25] && !ip.options_payload[26] && ip.options_payload[27] && !ip.options_payload[28] 
        && !ip.options_payload[29] && !ip.options_payload[31] && !ip.options_payload[32])
        {
            port80 = true;
        }
        
        if(direction==OUTGOING && port443)
        {
            response = DROP;
           
        }
        

        if(direction == OUTGOING && port80)
        {
            counter_tcp = counter_tcp+1;
            if(counter_tcp > 1)
            {
                response = DROP;
                counter_tcp = 0;
            
            }
        }
    }




    return response;
}
