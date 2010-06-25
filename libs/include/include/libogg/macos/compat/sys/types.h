#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__ 1

#include <MacTypes.h>
#include <alloca.h>
#include <string.h>

typedef short		int16_t;
typedef long		int32_t;
typedef long long	int64_t;

#define vorbis_size32_t long


#if defined(__cplusplus)
extern "C"
  {
#endif

#pragma options align=power

    char *strdup(const char *inStr);

#pragma options align=reset

#if defined(__cplusplus)
  }
#endif

#endif /* __SYS_TYPES_H__ */
