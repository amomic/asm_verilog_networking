#include "framework.h"

static AVContext* latestContext;

void signal_handler(int signum)
{
    printf("\nReceived SIGINT\n");
    deleteAVContext(latestContext);
    latestContext = nullptr;
    exit(0);
}

#define PORT (8554)

// Forward declaration of the handleConnection function. This function will be
// implemented by the students.
void handleConnection(int fd, const char *remote_addr, uint16_t remote_port);



MessageType stringToMessageType(const char *s)
{   
    int index = 0;
    for(; index < MessageType::NUM_MESSAGES; index++)
    {
        if(strcmp(s, messageTypes[index]) == 0) break;
    }
    return (MessageType)index;
}

const char* messageTypeToString(MessageType type)
{
    if(type >= MessageType::NUM_MESSAGES)
    {
        return messageTypes[MessageType::NUM_MESSAGES];
    }
    return messageTypes[(int)type];
}

const char *statusToString(StatusCode status)
{
    if(status > StatusCode::NotImplemented)
    {
        return nullptr;
    }
    return statusDescriptions[(int)status];
}



void setup_read(read_info* info, int fd)
{
    assert(info);
    info->fd = fd;
    for(int i = 0; i < MAX_LINES; i++)
    {
        info->lines[i] = nullptr;
    }
    info->numlines = 0;

}

void complete_read(read_info* info)
{
    for(size_t i = 0; i < info->numlines; i++)
    {
        free(info->lines[i]);
        info->lines[i] = nullptr;
    }
    info->numlines = 0;
}

void read_all_lines(read_info *info)
{
    complete_read(info);
    char buf[4096];
    memset(buf, 0, 4096);
    read(info->fd, buf, 4096);
    char *line = strtok(buf, "\r\n");
    info->numlines = 0;
    while(line != NULL && strlen(line) > 0)
    {
        size_t len = strlen(line)+1;
        info->lines[info->numlines] = (char*)malloc(len);
        strcpy(info->lines[info->numlines], line);
        info->numlines++;
        line = strtok(NULL, "\r\n");
    }
    for(size_t i = 0; i < info->numlines; i++)
    {
        printf("Line %ld : %s\n", i, info->lines[i]);
    }
}

char *search_for_header(read_info *info, const char* key)
{
    char *result = nullptr;
    for(size_t i = 0; i < info->numlines; i++)
    {
        if(!strncmp(key, info->lines[i], strlen(key)))
        {
            result = info->lines[i];
            break;
        }
    }
    return result;
}

const char *filename_from_path(const char *path)
{
    const char *res = nullptr;
    res = strrchr(path, '/');
    if(res) res++;
    return res;
}
bool fileExists(const char *filename)
{
    if(access(filename, F_OK) == 0)
    {
        return true;
    }
    return false;
}

int getSDPInfo(const char *filename, char *buffer, uint64_t size)
{
    if (buffer == nullptr || filename == nullptr || size < 2)
    {
        return -1;
    }
    AVFormatContext *context = NULL;
    avformat_open_input(&context, filename, NULL, NULL);
    if (context == nullptr)
    {
        return -1;
    }
    memset(buffer, 0, size);
    avformat_find_stream_info(context, nullptr);
    printf("Found %d stream(s)\n", context->nb_streams);
    av_sdp_create(&context, 1, buffer, size);
    printf("SDP: %s\n", buffer);
    avformat_close_input(&context);
    avformat_free_context(context);
    return 0;
}

AVContext *createAVContext(const char *filename, int client_port)
{
    AVContext *context = (AVContext*)malloc(sizeof(AVContext));
    latestContext = context;
    context->filename = (char*)malloc(strlen(filename)+1);
    strcpy(context->filename, filename);
    AVFormatContext *inputCtx = nullptr;
    avformat_open_input(&inputCtx, filename, nullptr, nullptr);
    if(!inputCtx)
    {
        perror("Could not create input context\n");
        exit(-1);
    }
    avformat_find_stream_info(inputCtx, nullptr);
    AVCodec *inputDecoder;
    int audioStreamPos = av_find_best_stream(inputCtx, AVMediaType::AVMEDIA_TYPE_AUDIO, -1, -1, &inputDecoder, 0);
    if (audioStreamPos < 0)
    {
        switch (audioStreamPos)
        {
        case AVERROR_STREAM_NOT_FOUND:
            fprintf(stderr, "No appropriate stream found\n");
            exit(-1);
        case AVERROR_DECODER_NOT_FOUND:
            fprintf(stderr, "Failed to decode audio stream\n");
            exit(-1);
        default:
            fprintf(stderr, "Failed for unknown reason (%d)\n", audioStreamPos);
            exit(-1);
        }
    }
    AVStream *inputAudioStream = inputCtx->streams[audioStreamPos];
    printf("Got %d streams overall\n", inputCtx->nb_streams);

    printf("Audio found: #%d %s\n", audioStreamPos, inputDecoder->long_name);
    AVOutputFormat *outputFmt = av_guess_format("rtp", nullptr, nullptr);
    AVFormatContext *outputCtx;
    char rtp_url[1024];
    sprintf(rtp_url, "rtp://127.0.0.1:%d", client_port);
    avformat_alloc_output_context2(&outputCtx, NULL, "rtp", rtp_url);
    if (!(outputFmt->flags & AVFMT_NOFILE))
    {
        avio_open(&outputCtx->pb, outputCtx->url, AVIO_FLAG_WRITE);
    }
    assert(outputCtx != nullptr);
    AVStream *outputAudioStream = avformat_new_stream(outputCtx, inputDecoder);
    avcodec_parameters_copy(outputAudioStream->codecpar, inputAudioStream->codecpar);
    avformat_transfer_internal_stream_timing_info(outputCtx->oformat, outputAudioStream, inputAudioStream, AVTimebaseSource::AVFMT_TBCF_AUTO);
    outputAudioStream->duration = inputAudioStream->duration;
    outputAudioStream->nb_frames = inputAudioStream->nb_frames;
    
    if(avformat_write_header(outputCtx, nullptr) < 0)
    {
        printf("Error when writing header!\n");
        exit(-1);
    }
    context->inputContext = inputCtx;
    context->outputContext = outputCtx;
    context->inputStream = inputAudioStream;
    context->outputStream = outputAudioStream;
    context->stream_pos = audioStreamPos;
    return context;
}

void deleteAVContext(AVContext *context)
{
    if(context == nullptr)
    {
        return;
    }
    if(context->filename) 
    {
        free(context->filename);
        context->filename = nullptr;
    }
    if(context->inputContext)
    {

        avformat_close_input(&context->inputContext);
        avformat_free_context(context->inputContext);
        context->inputContext = nullptr;
    }
    if(context->outputContext)
    {
        // We must include this line of code to avoid
        // leaking memory from "write_header"
        av_write_trailer(context->outputContext);
        avio_close(context->outputContext->pb);
        avformat_free_context(context->outputContext);
        context->outputContext = nullptr;
    }
    context->inputStream = nullptr;
    context->outputStream = nullptr;
    latestContext = nullptr;
    free(context);
}

int readPacketFromContext(AVContext *context, AVPacket *packet)
{
    if(context == nullptr || packet == nullptr)
    {
        return -1;
    }
    return av_read_frame(context->inputContext, packet);
}

void rescalePacketTimestamps(AVContext *context, AVPacket *packet)
{
    if(context == nullptr || packet == nullptr) return;
    packet->pts = av_rescale_q(packet->pts, context->inputStream->time_base, context->outputStream->time_base);
    packet->dts = av_rescale_q(packet->dts, context->inputStream->time_base, context->outputStream->time_base);
}

void sendAndFreePacket(AVContext *context, AVPacket *packet)
{
    av_write_frame(context->outputContext, packet);
    av_packet_unref(packet);
}

int main()
{

    signal(SIGINT, signal_handler);
    // We use a pre-defined port for our connection
    // const unsigned short PORT = 8554;
    // Create a new socket, AF_INET tells the socket to use IPv4
    // With SOCK_STREAM we tell the socket to use a
    // sequenced, reliable, two-way, connection-based byte stream
    // The last argument specifies the actual protocol, in our case
    // we use TCP. For more info see https://man7.org/linux/man-pages/man2/socket.2.html
    int listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listener == -1)
    {
        perror("Could not create socket");
        exit(1);
    }
    // Set additional options for the created socket
    // see https://man7.org/linux/man-pages/man7/socket.7.html for possible settings
    int opt = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    sockaddr_in listen_info;
    listen_info.sin_family = AF_INET;
    listen_info.sin_port = htons(PORT);
    listen_info.sin_addr.s_addr = INADDR_ANY;

    // Bind socket to local address, see https://man7.org/linux/man-pages/man2/bind.2.html
    int result;
    result = bind(listener, (sockaddr *)(&listen_info), sizeof(sockaddr_in));
    if (result < 0)
    {
        perror("Failed to bind to address");
        exit(2);
    }
    // Prepare the socket to accept connections
    result = listen(listener, 10);
    if (result < 0)
    {
        perror("Failed to switch socket to listen mode");
        exit(3);
    }
    sigset_t sigpipe_set;
    sigemptyset(&sigpipe_set);
    sigaddset(&sigpipe_set, SIGPIPE);
    sigprocmask(SIG_BLOCK, &sigpipe_set, NULL);
    printf("Server startup ok - listening on port %u.\n", PORT);
    do
    {
        sockaddr_in peer_info;
        socklen_t len = sizeof(sockaddr_in);
        int connection = accept(listener, (sockaddr *)(&peer_info), &len);
        if (connection < 0)
        {
            perror("Failed to accept incoming connection on socket");
            exit(4);
        }
        char *addr = inet_ntoa(peer_info.sin_addr);
        char buf[16];
        strncpy(buf, addr, 16);
        buf[15] = '\0';
        uint16_t port = ntohs(peer_info.sin_port);
        printf("Incoming connection from %s:%u...\n", &buf[0], port);
        handleConnection(connection, &buf[0], port);
        printf("Done handling connection\n");
    } while (true);
    return 0;
}