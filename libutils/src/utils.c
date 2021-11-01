#include "utils.h"

#include <stdint.h>
#include <time.h>
#include <errno.h>

int msleep(int msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

uint64_t getTicksMs(void)
{
    clock_t now = clock();
    uint64_t ret = (now * 1000000) / CLOCKS_PER_SEC;
    return ret;
}