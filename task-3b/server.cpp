#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>

#include <sys/sendfile.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "framework.h"
#define BONUS_TASK
#define RTSP_VERSION ("RTSP/1.0")


// Handle a server-client connection. This method is called 
// for each separate connection
void handleConnection(int fd, const char *remote_addr, uint16_t remote_port)
{
    read_info info;
    MessageType currentMessage = INVALID;
    AVContext *context = nullptr;
    // TODO: find a way to generate session ids
    //int session_id = rand();
    //int client_port = remote_port;
    // Initially, the state is set to initialization
    // Refer to the assignment description for the state logic
    //ConnectionState currentState = INITIALIZATION;
    int cseq;
    while(true)
    {
        // Setup the read operation and read
        // all lines from the socket
        printf("------------------\n");
        setup_read(&info, fd);
        read_all_lines(&info);
        printf("Read %ld lines\n", info.numlines);
        // Parse the method and the path from the first line
        char* method = info.lines[0];
        if (!method)
            goto empty_read;
        char* methodEnd = strchr(method, ' ');
        if (!methodEnd)
            goto bad_request;
        char* pathEnd = strrchr(methodEnd+1, ' ');
        if (!pathEnd)
            goto bad_request;
        (*methodEnd) = '\0';
        (*pathEnd) = '\0';
        char *path = methodEnd+1;
        // Extract information from the read lines
        currentMessage = stringToMessageType(method);
        printf("The current message type is %s\n", messageTypeToString(currentMessage));
        printf("Path is %s\n",path);
        // Get the CSeq value from the received message
        char *CSeq = search_for_header(&info, "CSeq:");
        if (!CSeq)
            goto missing_cseq;
        CSeq = strchr(CSeq, ' ');
        if (!CSeq)
            goto missing_cseq;
        cseq = atoi(CSeq);
        printf("CSeq is %d\n",cseq);
        switch(currentMessage)
        {
            case OPTIONS:
            {
// TODO: Implement this method
         
                dprintf(fd,"RTSP/1.0 %d %s\r\nCSeq: %d\r\nPublic: %s, %s, %s, %s, %s\r\n", statusCodes[0],statusDescriptions[0],cseq,messageTypes[0],messageTypes[1],messageTypes[2],messageTypes[3], messageTypes[5]);
                break;
            }
            case DESCRIBE:
            {
// TODO: Implement this method!
                void* buf;
                buf = av_mallocz(1024);
                size_t info = getSDPInfo(filename_from_path(path), (char*)buf, sizeof(buf));
                
               
                dprintf(fd,"RTSP/1.0 %d %s\r\nCSeq: %d\r\nContent-Type: application/sdp\r\nContent-Length: %zu\r\n\r\nm=audio 0 RTP/AVP 14\r\no=- 0 0 IN IP4 127.0.0.1", statusCodes[0],statusDescriptions[0],cseq,sizeof(info));
                break;
            } 
            case SETUP:
            {
// TODO: Implement this method!
                break;
            }
            case PLAY:
            {
// TODO: Implement this method!
                break;
            } 
            case PAUSE:
            {
// TODO: Implement this method!
                break;
            }
            case TEARDOWN:
            {
// TODO: Implement this method!
                break;
            }
            default:
            {
                printf("Unknown message type\n");
            }
        }
        complete_read(&info);
    }
    printf("Done receiving\n");
    goto close;
    empty_read:
        printf("Client disconnected\n");
        goto close;
    bad_request:
        printf("Invalid request\n");
        goto close;
    missing_cseq:
        printf("Missing CSeq header\n");
        goto close;
    close:
        complete_read(&info);
        close(fd);
        deleteAVContext(context);
        context = nullptr;
        return;
}


