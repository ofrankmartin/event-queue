#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>

/**
 * @brief Sleeps the current thread with msec precision
 * 
 * @param msec The amount of milliseconds to sleep
 * @return int 0 if success, -1 on failure 
 */
int msleep(int msec);

/**
 * @brief Get the milliseconds elapsed since the app started
 * 
 * @return uint64_t The milliseconds elapsed since the app started
 * 
 * @remark This is not a very precise way to count time, but it is enough
 */
uint64_t getTicksMs(void);

#endif // __UTILS_H__