#pragma once

#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

namespace platform::wayland::shm
{

inline void
randName(char* buf)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    long r = ts.tv_nsec;

    for (int i = 0; i < 6; ++i)
    {
        buf[i] = 'A' + (r & 15) + (r & 16) * 2;
        r >>= 5;
    }
}

inline int
createFile(void)
{
    int retries = 100;

    do
    {
        char name[] = "/wl_shm-XXXXXX";
        randName(name + sizeof(name) - 7);
        --retries;
        int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, 0600);
        if (fd >= 0)
        {
            shm_unlink(name);
            return fd;
        }
    }
    while (retries > 0 && errno == EEXIST);

    return -1;
}

inline int
allocFile(size_t size)
{
    int fd = createFile();
    if (fd < 0)
        return -1;
    int ret;

    do
    {
        ret = ftruncate(fd, size);
    }
    while (ret < 0 && errno == EINTR);

    if (ret < 0)
    {
        close(fd);
        return -1;
    }

    return fd;
}

} /* namespace platform::wayland::shm */
