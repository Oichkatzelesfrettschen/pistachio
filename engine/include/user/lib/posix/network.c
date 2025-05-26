#include "posix.h"
#include <sys/socket.h>
#include <arpa/inet.h>

int posix_setsockopt(int sockfd, int level, int optname,
                     const void *optval, socklen_t optlen)
{
    return setsockopt(sockfd, level, optname, optval, optlen);
}

int posix_getsockopt(int sockfd, int level, int optname,
                     void *optval, socklen_t *optlen)
{
    return getsockopt(sockfd, level, optname, optval, optlen);
}

int posix_inet_pton(int af, const char *src, void *dst)
{
    return inet_pton(af, src, dst);
}

const char *posix_inet_ntop(int af, const void *src, char *dst, socklen_t size)
{
    return inet_ntop(af, src, dst, size);
}
