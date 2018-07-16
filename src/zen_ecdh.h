#ifndef __ZEN_ECDH_H__
#define __ZEN_ECDH_H__

#include <zen_octet.h>
#ifndef STANDALONE
#include <pbc_support.h>
#define RNGTYPE cprng
#else
#include <random.h>
#define RNGTYPE WC_RNG
#define ECDH_INVALID_PUBLIC_KEY (-13)
#endif

typedef struct {
	// function pointers
	int (*ECP__KEY_PAIR_GENERATE)(RNGTYPE *R,octet *s,octet *W);
	int (*ECP__PUBLIC_KEY_VALIDATE)(octet *W);
	int (*ECP__SVDP_DH)(octet *s,octet *W,octet *K);
	void (*ECP__ECIES_ENCRYPT)(int h,octet *P1,octet *P2,
	                           RNGTYPE *R,octet *W,octet *M,int len,
	                           octet *V,octet *C,octet *T);
	int (*ECP__ECIES_DECRYPT)(int h,octet *P1,octet *P2,
	                          octet *V,octet *C,octet *T,
	                          octet *U,octet *M);
	int (*ECP__SP_DSA)(int h,RNGTYPE *R,octet *k,octet *s,
	                   octet *M,octet *c,octet *d);
	int (*ECP__VP_DSA)(int h,octet *W,octet *M,octet *c,octet *d);
	RNGTYPE *rng;
	int keysize;
	int fieldsize;
	int hash; // hash type is also bytes length of hash
	char curve[16]; // just short names
	char type[16];
	octet *pubkey;
	int publen;
	octet *seckey;
	int seclen;
} ecdh;

#endif
