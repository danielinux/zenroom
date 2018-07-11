
#include <ed25519.h>

typedef struct {
	char curve[16];
	char type[16];
	ed25519_key *ed25519;
} ecp;



