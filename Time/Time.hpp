#pragma once

#include <ctime>

namespace CuTime
{
    inline void Gmt(tm *gmt, const time_t *time)
    {
#if (defined _WIN32 || _WIN64)
        gmtime_s(gmt, time);
#else
        gmtime_r(time, gmt);
#endif
    }

    inline void Local(tm *local, const time_t *time)
    {
#if (defined _WIN32 || _WIN64)
        localtime_s(local, time);
#else
        localtime_r(time, local);
#endif
    }
}
