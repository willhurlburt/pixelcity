#ifdef WINDOWS
# include <windows.h>
#else
# include <sys/time.h>
# include <time.h>
#endif

#include "time_util.h"

int GetTimeInMillis()
{
#ifdef WINDOWS
  return GetTickCount();
#else
  struct timeval tv;

  gettimeofday(&tv, NULL);

  return tv.tv_usec / 1000 + ((tv.tv_sec % 1000000) * 1000);
#endif
}
