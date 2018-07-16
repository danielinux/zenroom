#ifndef sss_RANDOMBYTES_H
#define sss_RANDOMBYTES_H

#ifdef _WIN32
/* Load size_t on windows */
#include <crtdefs.h>
#else
#ifndef HAVE_CUSTOM_GETRANDOM
#include <sys/syscall.h>
#endif
#endif /* _WIN32 */


/*
 * Write `n` bytes of high quality random bytes to `buf`
 */
int randombytes(void *buf, size_t n);


#endif /* sss_RANDOMBYTES_H */
