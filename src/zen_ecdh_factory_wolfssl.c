#include <jutils.h>
#include <zen_ecdh.h>

#include <ed25519.h>
#include <curve25519.h>

/*
	int (*ECP__KEY_PAIR_GENERATE)(RNGTYPE R,octet *s,octet *W);
	int (*ECP__PUBLIC_KEY_VALIDATE)(octet *W);
	int (*ECP__SVDP_DH)(octet *s,octet *W,octet *K);
	void (*ECP__ECIES_ENCRYPT)(int h,octet *P1,octet *P2,
	                           RNGTYPE R,octet *W,octet *M,int len,
	                           octet *V,octet *C,octet *T);
	int (*ECP__ECIES_DECRYPT)(int h,octet *P1,octet *P2,
	                          octet *V,octet *C,octet *T,
	                          octet *U,octet *M);
	int (*ECP__SP_DSA)(int h,RNGTYPE R,octet *k,octet *s,
	                   octet *M,octet *c,octet *d);
	int (*ECP__VP_DSA)(int h,octet *W,octet *M,octet *c,octet *d);
*/

int wolfcrypt_ed25519_keypair_generate(WC_RNG *R, octet *s, octet *W)
{
    
    return 0;
}

int wolfcrypt_ed25519_public_key_validate(octet *W)
{

    return 0;
}

int wolfcrypt_ed25519_svdp_dh(octet *s, octet *W, octet *K)
{

    return 0;
}

void wolfcrypt_ed25519_ecies_encrypt(int h, octet *P1, octet *P2, WC_RNG *R, 
        octet *W, octet *M, int len, octet *V, octet *C, octet *T)
{

}

void wolfcrypt_ed25519_ecies_decrypt(int h, octet *P1, octet *P2,
       int len, octet *V, octet *C, octet *T, octet *U, octet *M)
{

}

int wolfcrypt_ed25519_sp_dsa(int h,RNGTYPE R,octet *k,octet *s,octet *M,octet *c,octet *d)
{
    return 0;
}

int wolfcrypt_ed25519_vp_dsa(int h, octet *W,octet *M,octet *c,octet *d)
{
    return 0;

}

ecdh *ecdh_new_curve(lua_State *L, const char *curve) {
	ecdh *e = NULL;
	if(strcasecmp(curve,"ec25519")   ==0
	   || strcasecmp(curve,"ed25519")==0
	   || strcasecmp(curve,"25519")  ==0) {
		e = (ecdh*)lua_newuserdata(L, sizeof(ecdh));
		e->keysize = CURVE25519_KEYSIZE; 
		e->fieldsize = 2*CURVE25519_KEYSIZE; 
		e->hash = SHA512_BLOCK_SIZE;
		e->ECP__KEY_PAIR_GENERATE = wolfcrypt_ed25519_keypair_generate;
		e->ECP__PUBLIC_KEY_VALIDATE	= wolfcrypt_ed25519_public_key_validate;
		e->ECP__SVDP_DH = wolfcrypt_ed25519_svdp_dh;
		e->ECP__ECIES_ENCRYPT = wolfcrypt_ed25519_ecies_encrypt;
		e->ECP__ECIES_DECRYPT = wolfcrypt_ed25519_ecies_decrypt;
		e->ECP__SP_DSA = wolfcrypt_ed25519_sp_dsa;
		e->ECP__VP_DSA = wolfcrypt_ed25519_vp_dsa;
		strncpy(e->curve,curve,15);
		strcpy(e->type,"edwards");
	} else  {
		error(L, "%s: curve not supported in standalone mode: %s",__func__,curve);
		return NULL;
	}
	return e;
}
