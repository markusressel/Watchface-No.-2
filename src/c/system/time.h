#pragma once

#include <pebble.h>

#ifndef MOCK_TIME
/**
 * Custom wrapper for time() that returns either the system time or the simulated time
 * from developer options if UseSimulatedTime is set.
 * @param tloc optional pointer to store the time
 * @return the current (simulated or real) time
 */
time_t custom_time(time_t *tloc);

/**
 * Custom wrapper for localtime() that converts a time_t pointer to a struct tm.
 * @param timep pointer to the time_t to convert
 * @return the struct tm representation
 */
struct tm *custom_localtime(const time_t *timep);
#endif
