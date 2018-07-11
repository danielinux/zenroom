#include <stdint.h>
		
#define NO_OLD_RNGNAME  
#define SMALL_SESSION_CACHE
#define WOLFSSL_SMALL_STACK

#define SINGLE_THREADED
#define NO_SIG_WRAPPER
		
/* Cipher features */
//#define USE_FAST_MATH
#define NO_FILESYSTEM
#define NO_DEVURANDOM

#define HAVE_FFDHE_2048
#define HAVE_CHACHA 
#define HAVE_POLY1305 
#define HAVE_ECC 
#define HAVE_CURVE25519
#define CURVED25519_SMALL
#define HAVE_ONE_TIME_AUTH
#define WOLFSSL_DH_CONST
#define WORD64_AVAILABLE
		

#define HAVE_ED25519
#define HAVE_POLY1305
#define HAVE_SHA512
#define WOLFSSL_SHA512

/* Robustness */
#define TFM_TIMING_RESISTANT
#define ECC_TIMING_RESISTANT
#define WC_RSA_BLINDING

/* Remove Features */
#define NO_WRITEV
#define NO_MD4
#define NO_RABBIT
#define NO_HC128

#include <stdlib.h>
