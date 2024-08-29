#ifndef INTRINSICS_H
#define INTRINSICS_H

#ifdef __cplusplus
extern "C" {
#endif

int __cntlzw(unsigned int n);
double __fabs(double x);
double __frsqrte(double x);
int __rlwimi(int a, int b, int c, int d, int e);
void __sync(void);

#ifdef __cplusplus
}
#endif

#endif
