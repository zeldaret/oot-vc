#ifndef _ASSERT_H
#define _ASSERT_H

#ifdef __cplusplus
extern "C" {
#endif

void __msl_assertion_failed(const char*, const char*, const char*, int);

#define assert(condition) ((condition) ? ((void)0) : __msl_assertion_failed(#condition, __FILE__, 0, __LINE__))

#ifdef __cplusplus
}
#endif

#endif
