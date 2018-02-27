# Zenroom - DECODE project

[![software by Dyne.org](https://www.dyne.org/wp-content/uploads/2015/12/software_by_dyne.png)](http://www.dyne.org)

[![Build Status](https://travis-ci.org/DECODEproject/zenroom.svg?branch=master)](https://travis-ci.org/DECODEproject/zenroom)

**Restricted execution environment** for cryptographic operations in a **Turing-incomplete language** based on syntax-direct translation and coarse-grained control of computations and memory used.

Zenroom is a POSIX portable language interpreter inspired by language-theoretical security and it is designed to be small, attack-resistant and very portable. Its main use case is **distributed computing** of untrusted code, for instance it can be used in any distributed ledger implementation (also known as **blockchain smart contracts**).

Zenroom compiles to **static native binaries** (ARM and X86, UNIX and Windows64) for embedded use, but also to **javascript and webassembly** for client-side usage for instance in **browsers and mobile applications**.

With Zenroom is easy to write **portable** software using **end-to-end encryption** inside isolated environments instead of adding built-in dependencies and applications adopting Zenroom for cryptographic computations can be easily made **interoperable**. Basic crypto functions provided include primitives from AES and soon CAESAR competition winners to manage **a/symmetric keys, key derivation, hashing and signing functionalities**.

For a larger picture describing the purpose and design of this software in the field of **data-ownership** and **secure distributed computing**, see:

- The DECODE Project website: https://decodeproject.eu
- The DECODE Whitepaper: https://decodeproject.github.io/whitepaper
- The Zenroom design documentation: https://decodeproject.github.io/zenroom

![Horizon 2020](docs/ec_logo.png)

This project is receiving funding from the European Union’s Horizon 2020 research and innovation programme under grant agreement nr. 732546 (DECODE).

## Build instructions

Pre-built binaries are available here https://sdk.dyne.org:4443/view/decode/ - this section is optional for those who want to build this software from source. The following build instructions contain generic information meant for an expert audience.

The Zenroom compiles the same sourcecode to run on Linux in the form of 2 different POSIX compatible ELF binary formats using GCC (linking shared libraries) or musl-libc (fully static) targeting both X86 and ARM architectures. It also compiles to a Windows 64-bit native and fully static executable. At last, it compiles to Javascript/Webassembly using the LLVM based emscripten SDK. To recapitulate some Makefile targets:

1. `make shared` its the simpliest, builds a shared executable linked to a system-wide libc, libm and libpthread (mostly for debugging)
2. `make static` builds a fully static executable linked to musl-libc (to be operated on embedded platforms)
3. `make js` or `make wasm` builds different flavors of Javascript modules to be operated from a browser or NodeJS (for client side operations)
4. `make win` builds a Windows 64bit executable with no DLL dependancy, containing the LUA interpreter and all crypto functions (for client side operations on windows desktops)

Remember that if after cloning this source code from git, one should do:
```
git submodule update --init --recursive
```

Then first build the shared executable environment:

```
make shared
```
To run tests:

```
make check-shared
```

To build the static environment:

```
make bootstrap
make static
make check-static
```

For the Javascript and WebAssembly modules the Zenroom provides various targets provided by emscripten which must be installed and loaded in the environment according to the emsdk's instructions :

```
make js
make wasm
make html
```

## Crypto functionalities

The Zenroom language interpreter includes statically the following cryptographic primitives:

- **Norx** authenticated encryption with additional data (AEAD) - this is the default 64-4-1 variant (256-bit key and nonce, 4 rounds)
- **Blake2b** cryptographic hash function
- **Argon2i**, a modern key derivation function based on Blake2. Like
scrypt, it is designed to be expensive in both CPU and memory.
- **Curve25519**-based key exchange and public key encryption,
- **Ed25519**-based signature function using Blake2b hash instead of sha512,

Legacy cryptographic functions include **md5**, and **rc4**.

Endoding and decoding functions are provided for **base64** and **base58** (for base58, the BitCoin encoding alphabet is used).

Compression functions based on **BriefLZ** are also included.

Inclusion of classic NIST compliant **RSA** and **DSA** algorithms is in progress.

## Operating instructions

This software is work in progress and this section will be extended in
the near future. Scripts found in the test/ directory provide good
examples to start from.

From **command-line** the Zenroom is operated passing files as
arguments:

```
Usage: zenroom [-c config] [-k keys] script.lua
```

From **javascript** the function `zenroom_exec()` is exposed with four
arguments: three strings and one number from 1 to 3 indicating the
verbosity of output on the console:

```
int zenroom_exec(char *script, char *config, char *keys, char *data, int verbosity)
```

The contents of the three strings cannot exceed 100k in size and are of different types:

- `script` is a parsable LUA script, for example:
```lua
t = "The quick brown fox jumps over the lazy dog"
pk, sk = keygen_sign_ed25519() -- signature keypair
sig = sign_ed25519(sk, pk, t)
assert(#sig == 64)
assert(check_ed25519(sig, pk, t))
```

- `config` is also a parsable LUA script declaring variables, for example:
```lua
memory_limit = 100000
instruction_limit = 696969
output_limit = 64*1024
log_level = 7
remove_entries = {
	[''] = {'dofile','load', 'loadfile','newproxy'},
	os = {'getenv','execute','exit','remove','rename',
		  'setlocale','tmpname'},
    math = {'random', 'randomseed'}
 }
disable_modules = {io = 1}
```

- `arguments` is a simple string, but can be also a json map used to
  pass multiple arguments

For example create a json file containing a map (this can be a string
passed from javascript)

```json
{
	"secret": "zen and the art of programming",
	"salt": "OU9Qxl3xfClMeiCz"
}
```

Then run `zenroon -a arguments.json` and pass the following script as
final argument, or pipe from stdin or passed as a string argument to
`zenroom_exec()` from javascript:

```lua
json = cjson()
args = json.decode(arguments)
-- args is now a lua table containing values for each args.argname
print(args.secret)
print(args.salt)
```

All strings parsed are in the `arguments` global variable available
inside the script. This allows separation of public code and private
data to be passed via separate channels.

So for instance if we want to encrypt a secret message for multiple
recipients who have provided us with their public keys, one would load
this example keyfile:

```json
{
    "keyring": {
        "public":"GoTdVYbTWEoZ4KtCeMghV7UcpJyhvny1QjVPf8K4oi1i",
        "secret":"9PSbkNgsbgPnX3hM19MHVMpp2mzvmHcXCcz6iV8r7RyZ"
    },
    "recipients": {
        "jaromil": "A1g6CwFobiMEq6uj4kPxfouLw1Vxk4utZ2W5z17dnNkv",
        "francesca": "CQ9DE4E5Ag2e71dUW2STYbwseLLnmY1F9pR85gLWkEC6",
        "jimb": "FNUdjaPchQsxSjzSbPsMNVPA2v1XUhNPazStSRmVcTwu",
        "mark": "9zxrLG7kwheF3FMa852r3bZ4NEmowYhxdTj3kVfoipPV",
        "paulus": "2LbUdFSu9mkvtVE6GuvtJxiFnUWBWdYjK2Snq4VhnzpB",
        "mayo": "5LrSGTwmnBFvm3MekxSxE9KWVENdSPtcmx3RZbktRiXc"
    }
}
```

And then with this code:

```lua
secret="this is a secret that noone knows"
-- this should be a random string every time
nonce="eishai7Queequot7pooc3eiC7Ohthoh1"

json = cjson_safe()
keys = json.decode(arguments)

res = {}

for name,pubkey in pairs(keys.recipients) do
   k = exchange_session_x25519(
	  decode_b58(keys.keyring.secret),
	  decode_b58(pubkey))
   enc = encrypt_norx(k,nonce,secret)
   -- insert in results
   res[name]=encode_b58(enc)
end
print(json.encode(res))
```

Zenroom can be executed as `zenroom -k keys.json code.lua` and will print out the encrypted message for each recipient reorganised in a similar json structure:

```json
{
   "jaromil" : "Ha8185xZoiMiJhfquKRHvtT6vZPWifGaXmD4gxjyfHV9ASNaJF2Xq85NCmeyy4hWLGns4MTbPsRmZ2H7uJh9vEuWt",
   "mark" : "13nhCBWKbPAYyhJXD7aeHtiFKb89fycBnoKy2nosJdSqfS2vhhHqBvVKb2oasiga9P3UyaEJZQdyYRfiBBKEswdmQ",
   "francesca" : "7ro9u2ViXjp3AaLzvve4E4ebZNoBPLtxAja8wd8YNn51TD9LjMXNGsRvm85UQ2vmhdTeJuvcmBvz5WuFkdgh3kQxH",
   "mayo" : "FAjQSYXZdZz3KRuw1MX4aLSjky6kbpRdXdAzhx1YeQxu3JiGDD7GUFK2rhbUfD3i5cEc3tU1RBpoK5NCoWbf2reZc",
   "jimb" : "7gb5SLYieoFsP4jYfaPM46Vm4XUP2jbCUnkFRQfwNrnJfqaew5VpwqjbNbLJrqGsgJJ995bP2867nYLcn96wuMDMw",
   "paulus" : "8SGBpRjZ21pgYZhXmy7uWGNEEN7wnHkrWtHEKeh7uCJgsDKtoGZHPk29itCV6oRxPbbiWEuN9Sm83jeZ1vinwQyXM"
}
```


## Acknowledgements

Copyright (C) 2017-2018 by Dyne.org foundation, Amsterdam

Designed, written and maintained by Denis Roio <jaromil@dyne.org>

Includes code by:

- Mozilla foundation (lua_sandbox)
- Rich Felker, et al (musl-libc)
- Mike Scott and Kealan McCusker (milagro)
- Phil Leblanc (luazen)
- Joergen Ibsen (brieflz)
- Loup Vaillant (blake2b, argon2i, ed/x25519)
- Samuel Neves and Philipp Jovanovic (norx)
- Luiz Henrique de Figueiredo (base64)
- Luke Dashjr (base58)
- Cameron Rich (md5)
- Mark Pulford (lua-cjson)
- Daan Sprenkels (randombytes)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License version 2 as
published by the Free Software Foundation.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
