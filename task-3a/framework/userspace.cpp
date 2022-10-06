#include <netinet/in.h>
#include <linux/types.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <functional>
#include <cstdio>
#include <csignal>
#include <unistd.h>
#include <sys/types.h>

#include "userspace.h"

struct scope_exit_guard
{
    template <typename T>
    scope_exit_guard(T&& fn) : _guard(std::forward<T>(fn)) {}
    ~scope_exit_guard() { _guard(); }

    private:
        std::function<void()> const _guard;
};

static int filter_fn(
    nfq_q_handle* qh,
    nfgenmsg* nfmsg,
    nfq_data* nfa,
    void* data
)
{
    nfqnl_msg_packet_hdr* ph = nfq_get_msg_packet_hdr(nfa);
    if (!ph)
    {
        fprintf(stderr, "failed: nfq_get_msg_packet_hdr\n");
        return 0;
    }

    int id = ntohl(ph->packet_id);

    uint8_t* pktData;
    int pktLen = nfq_get_payload(nfa, &pktData);
    if (pktLen <= 0)
    {
        fprintf(stderr, "failed: nfq_get_payload\n");
        return 0;
    }

    Direction const direction = (ph->hook == NF_INET_LOCAL_IN) ? INCOMING : OUTGOING;
    FilterResponse resp = processPacket(pktData, pktLen, direction);
    if (resp.accept)
    {
        if (resp.overridePacketData)
            return nfq_set_verdict(qh, id, NF_ACCEPT, resp.overridePacketData->size(), resp.overridePacketData->data());
        else
            return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
    }
    else
    {
        if (resp.overridePacketData)
        {
            // feel free to remove/comment out this message if you are intentionally triggering it (but why?)
            fprintf(stderr, "Your DROP response has override packet data set. The framework is ignoring this data. Are you sure this is intended?");
        }
        return nfq_set_verdict(qh, id, NF_DROP, 0, nullptr);
    }
}

#define CHECKED_CALL(fn) \
  { \
    int rv = (fn); \
    if (rv < 0) { \
      fprintf(stderr, "failed: %s\n", #fn); \
      exit(1); \
    } \
  }

template <typename T>
T __ptr__check(T ptr, char const* fnstr)
{
    if (!ptr)
        fprintf(stderr, "failed: %s\n", fnstr);
    return ptr;
}

#define CHECKED_CALL_PTR(fn) __ptr__check((fn), #fn)
  

static std::sig_atomic_t stop = 0;
int main(int argc, char const** argv)
{
    uid_t euid = geteuid();
    if (euid != 0)
    {
        fprintf(stderr, "I don't have root privileges.\n\nOnly root can configure net filtering.\nYou'll need to run me using sudo.\n");
        return 0;
    }
    nfq_handle* h = CHECKED_CALL_PTR(nfq_open());
    if (!h) return 1;
    scope_exit_guard gH([h]() { nfq_close(h); });

    CHECKED_CALL(nfq_unbind_pf(h, AF_INET));
    CHECKED_CALL(nfq_bind_pf(h, AF_INET));
    
    nfq_q_handle* qh = CHECKED_CALL_PTR(nfq_create_queue(h, 0, &filter_fn, nullptr));
    if (!qh) return 1;
    scope_exit_guard gQH([qh]() { nfq_destroy_queue(qh); });
    
    CHECKED_CALL(nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff));

    int fd = nfq_fd(h);
    if (fd < 0)
    {
        fprintf(stderr, "nfq_fd returned invalid fd\n");
        exit(1);
    }

    { // cleanly exit on SIGINT to keep valgrind happy
        struct sigaction act;
        act.sa_handler = [](int) { stop = 1; };
        sigemptyset(&act.sa_mask);
        act.sa_flags = SA_NODEFER;
        CHECKED_CALL(sigaction(SIGINT, &act, nullptr));
    }

    std::array<char, 0xffff> buf;
    int rlen;
    while (!stop && (rlen = recv(fd, buf.data(), buf.size(), 0)))
        nfq_handle_packet(h, buf.data(), rlen);

    printf("\nCTRL-C received, exiting...\n");
    return 0;
}
