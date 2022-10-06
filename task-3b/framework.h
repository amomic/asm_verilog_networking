#ifndef FRAMEWORK_H
#define FRAMEWORK_H
#include <arpa/inet.h>
#include <assert.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <vector>
#include <memory>
extern "C"
{
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
}
// Enum holding the current server state
// Each connection with a client has its own state
typedef enum ConnectionState
{
    INITIALIZATION,
    READY,
    PLAYING
} ConnectionState;

// The status codes that you will need to support
typedef enum StatusCode
{
    OK,
    NotFound,
    SessionNotFound,
    MethodNotValid,
    NotImplemented
} StatusCode;

// Enum for different message types.
// The NUM_MESSAGES type equals an INVALID message
typedef enum MessageType
{
    OPTIONS,
    DESCRIBE,
    SETUP,
    PLAY,
    PAUSE,
    TEARDOWN,
    NUM_MESSAGES,
    INVALID=NUM_MESSAGES
} MessageType;

// Context struct for packet streaming
typedef struct AVContext
{
    AVStream *inputStream;
    AVStream *outputStream;
    AVFormatContext *inputContext;
    AVFormatContext *outputContext;
    char *filename;
    int stream_pos;
} AVContext;

// Everything concerning reading from the client connection
// The maximum number of lines per message is fixed
#define MAX_LINES (32)
typedef struct
{
    int fd;
    char *lines[MAX_LINES];
    size_t numlines;
} read_info;

// Static maps that are used for conversion

static const char *messageTypes[] = {"OPTIONS","DESCRIBE", "SETUP", "PLAY", "PAUSE", "TEARDOWN", "INVALID"};

static const int statusCodes[] = {200, 404, 454, 455, 501};

static const char *statusDescriptions[] = {"OK", "Not Found", "Session Not Found", "Method Not Valid in This State", "Not Implemented"};
// Convert a string to a MessageType enum element
MessageType stringToMessageType(const char *s);
// Convert a MessageType enum to the corresponding string
const char* messageTypeToString(MessageType type);
// Convert a status code to a string
const char *statusToString(StatusCode status);
// Setup a read operation from file descriptor fd
void setup_read(read_info* info, int fd);
// Complete the read operation
void complete_read(read_info* info);
// Read all available lines. Reads up to 4096 characters
void read_all_lines(read_info *info);
// Search for a header defined by <key>
char *search_for_header(read_info *info, const char* key);
// Extract the filename from the path
const char *filename_from_path(const char *path);
// Check if a file exists
bool fileExists(const char *filename);
// Get the sdp info of <filename> and store <size> characters to <buffer>
// returns 0 on success.
int getSDPInfo(const char *filename, char *buffer, uint64_t size);
// Create a new output context.
// MUST be freed using the deleteAVContext function
AVContext *createAVContext(const char *filename, int client_port);
// Read a packet from the given context. 
// Returns 0 if the read is successful.
// Returns a non-zero value if no more data is available
int readPacketFromContext(AVContext *context, AVPacket *packet);
// Rescale the timestamps of the packet to fit the time base of the context
void rescalePacketTimestamps(AVContext *context, AVPacket *packet);
// Write the packet to the RTP stream and free its resources 
void sendAndFreePacket(AVContext *context, AVPacket *packet);
// Delete a previously allocated AVContext and free all encapsuled contexts and streams
void deleteAVContext(AVContext *context);
#endif