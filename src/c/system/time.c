#include "time.h"
#include "../developer_options.h"

static bool parse_time_string(const char *const str, int *const hour, int *const min, int *const sec) {
    if (str == NULL) {
        return false;
    }
    
    int h = 0;
    int m = 0;
    int s = 0;
    const char *p = str;
    
    // Parse hour
    if (*p < '0' || *p > '9') {
        return false;
    }
    while (*p >= '0' && *p <= '9') {
        h = h * 10 + (*p - '0');
        p++;
    }
    if (*p != ':') {
        return false;
    }
    p++;
    
    // Parse minute
    if (*p < '0' || *p > '9') {
        return false;
    }
    while (*p >= '0' && *p <= '9') {
        m = m * 10 + (*p - '0');
        p++;
    }
    if (*p != ':') {
        return false;
    }
    p++;
    
    // Parse second
    if (*p < '0' || *p > '9') {
        return false;
    }
    while (*p >= '0' && *p <= '9') {
        s = s * 10 + (*p - '0');
        p++;
    }
    
    if (h < 0 || h > 23 || m < 0 || m > 59 || s < 0 || s > 59) {
        return false;
    }
    
    *hour = h;
    *min = m;
    *sec = s;
    return true;
}

time_t custom_time(time_t *const tloc) {
    time_t t = time(NULL);
    if (DEV_OPTIONS.UseSimulatedTime && DEV_OPTIONS.SimulatedTime != NULL) {
        int hour = 0;
        int min = 0;
        int sec = 0;
        if (parse_time_string(DEV_OPTIONS.SimulatedTime, &hour, &min, &sec)) {
            const struct tm *const t_struct = localtime(&t);
            if (t_struct != NULL) {
                struct tm local_t = *t_struct;
                local_t.tm_hour = hour;
                local_t.tm_min = min;
                local_t.tm_sec = sec;
                t = mktime(&local_t);
            }
        }
    }
    if (tloc != NULL) {
        *tloc = t;
    }
    return t;
}

struct tm *custom_localtime(const time_t *const timep) {
    return localtime(timep);
}
