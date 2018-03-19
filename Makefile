pwd := $(shell pwd)
luasand := ${pwd}/build/lua_sandbox
mil := ${pwd}/build/milagro
extras := ${pwd}/docs/demo

# default
gcc := gcc
ar := ar
ranlib := ranlib
cflags_protection := -fstack-protector-all -D_FORTIFY_SOURCE=2 -fno-strict-overflow
cflags := -O2 ${cflags_protection}
musl := ${pwd}/build/musl
platform := posix

test-exec := ${pwd}/src/zenroom-shared -c ${pwd}/test/decode-test.conf

patches:
	./build/apply-patches

embed-lua:
	@echo "Embedding all files in src/lua"
	./build/embed-lualibs
	@echo "File generated: src/lualibs_detected.c"
	@echo "    and lualbs: src/lualib_*.c"
	@echo "Must commit to git if modified, see git diff."

# TODO: improve flags according to
# https://github.com/kripken/emscripten/blob/master/src/settings.js
js: gcc=${EMSCRIPTEN}/emcc
js: ar=${EMSCRIPTEN}/emar
js: cflags := --memory-init-file 0 -O2
js: ldflags := -s "EXPORTED_FUNCTIONS='[\"_zenroom_exec\"]'" -s "EXTRA_EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]'" -s ALLOW_MEMORY_GROWTH=1 -s USE_SDL=0
js: patches lua53 luazen
	CC=${gcc} CFLAGS="${cflags}" LDFLAGS="${ldflags}" make -C src js

wasm: gcc=${EMSCRIPTEN}/emcc
wasm: ar=${EMSCRIPTEN}/emar
wasm: cflags := -O2
wasm: ldflags := -s WASM=1 -s "EXPORTED_FUNCTIONS='[\"_zenroom_exec\"]'" -s "EXTRA_EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]'" -s MODULARIZE=1
wasm: patches lua53 luazen
	CC=${gcc} CFLAGS="${cflags}" LDFLAGS="${ldflags}" make -C src js

demo: gcc=${EMSCRIPTEN}/emcc
demo: ar=${EMSCRIPTEN}/emar
demo: cflags := -O2
demo: ldflags := -s WASM=1 -s "EXPORTED_FUNCTIONS='[\"_zenroom_exec\"]'" -s "EXTRA_EXPORTED_RUNTIME_METHODS='[\"ccall\",\"cwrap\"]'" -s ASSERTIONS=1 --shell-file ${extras}/shell_minimal.html -s NO_EXIT_RUNTIME=1 -s USE_SDL=0 -s USE_PTHREADS=0
demo: patches lua53 luazen
	CC=${gcc} CFLAGS="${cflags}" LDFLAGS="${ldflags}" make -C src demo

html: gcc=${EMSCRIPTEN}/emcc
html: ar=${EMSCRIPTEN}/emar
html: cflags := -O2
html: ldflags := -sEXPORTED_FUNCTIONS='["_main","_zenroom_exec"]'
html: patches  lua53 luazen
	CC=${gcc} CFLAGS="${cflags}" LDFLAGS="${ldflags}" make -C src html

win: gcc=x86_64-w64-mingw32-gcc
win: ar=x86_64-w64-mingw32-ar
win: ranlib=x86_64-w64-mingw32-ranlib
win: platform = posix
win: patches lua53 luazen milagro-win
	CC=${gcc} CFLAGS="${cflags}" make -C src win

static: gcc := musl-gcc
static: cflags := -Os -static -Wall -std=gnu99 ${cflags_protection}
static: ldflags := -static
static: patches lua53 luazen milagro
	CC=${gcc} CFLAGS="${cflags}" LDFLAGS="${ldflags}" make -C src static

system-static: cflags := -Os -static -Wall -std=gnu99 ${cflags_protection}
system-static: ldflags := -static
system-static: patches lua53 luazen milagro
	CC=${gcc} CFLAGS="${cflags}" LDFLAGS="${ldflags}" make -C src system-static

shared: gcc := gcc
shared: cflags := -O2 -fPIC ${cflags_protection}
shared: ldflags := -lm
shared: platform := linux
shared: patches lua53 luazen
	CC=${gcc} CFLAGS="${cflags}" make -C src shared


osx: gcc := gcc
osx: cflags := -O2 -fPIC ${cflags_protection}
osx: ldflags := -lm
osx: platform := macosx
osx: patches lua53 luazen
	CC=${gcc} CFLAGS="${cflags}" make -C src shared

debug: gcc := gcc
debug: cflags := -O0 -ggdb
debug: patches lua53 luazen
	CC=${gcc} CFLAGS="${cflags}" make -C src shared

lua53:
	CC=${gcc} CFLAGS="${cflags} \
	-DLUA_COMPAT_5_3 -DLUA_COMPAT_MODULE" \
	LDFLAGS="${ldflags}" AR="${ar}" RANLIB=${ranlib} \
	make -C ${pwd}/lib/lua53/src ${platform}

gmp:
	cd ${pwd}/lib/gmp && CFLAGS="${cflags}" CC=${gcc} ./configure --disable-shared
	make -C ${pwd}/lib/gmp

pbc:
	mkdir -p ${pwd}/build/pbc
	if ! [ ${pwd}/build/pbc/.libs/libpbc.a ]; then cd ${pwd}/build/pbc && CFLAGS="${cflags}" CC=${gcc} ${pwd}/lib/pbc/configure --disable-shared &&	make -C ${pwd}/build/pbc LDFLAGS="-L${pwd}/lib/gmp/.libs -l:libgmp.a" CFLAGS="${cflags} -I${pwd}/lib/gmp -I${pwd}/lib/pbc/include"; return 0; fi

luazen:
	CC=${gcc} AR=${ar} CFLAGS="${cflags}" make -C ${pwd}/build/luazen

milagro:
	@echo "-- Building milagro"
	if ! [ -r ${pwd}/lib/milagro-crypto-c/CMakeCache.txt ]; then cd ${pwd}/lib/milagro-crypto-c && CC=${gcc} cmake . -DBUILD_SHARED_LIBS=OFF -DBUILD_PYTHON=OFF -DBUILD_DOXYGEN=OFF -DCMAKE_C_FLAGS="${cflags}" -DAMCL_CHUNK=32 -DAMCL_CURVE=ED25519 -DAMCL_RSA=2048; fi
	if ! [ -r ${pwd}/lib/milagro-crypto-c/lib/libamcl_core.a ]; then CC=${gcc} CFLAGS="${cflags}" make -C ${pwd}/lib/milagro-crypto-c; fi

milagro-win:
	@echo "-- Building milagro"
	sed -i 's/project (AMCL)/project (AMCL C)/' ${pwd}/lib/milagro-crypto-c/CMakeLists.txt
	if ! [ -r ${pwd}/lib/milagro-crypto-c/CMakeCache.txt ]; then cd ${pwd}/lib/milagro-crypto-c && CC=${gcc} cmake . -DBUILD_SHARED_LIBS=OFF -DBUILD_PYTHON=OFF -DBUILD_DOXYGEN=OFF -DCMAKE_C_FLAGS="${cflags}" -DAMCL_CHUNK=32 -DAMCL_CURVE=ED25519 -DAMCL_RSA=2048 -DCMAKE_SHARED_LIBRARY_LINK_FLAGS="" -DCMAKE_SYSTEM_NAME="Windows"; fi
	if ! [ -r ${pwd}/lib/milagro-crypto-c/lib/libamcl_core.a ]; then CC=${gcc} CFLAGS="${cflags}" make -C ${pwd}/lib/milagro-crypto-c; fi

check-milagro: milagro
	CC=${gcc} CFLAGS="${cflags}" make -C ${pwd}/lib/milagro-crypto-c test

check-shared: test-exec := ${pwd}/src/zenroom-shared
check-shared: check-milagro
	@${test-exec} test/vararg.lua && \
	${test-exec} test/pm.lua && \
	${test-exec} test/nextvar.lua && \
	${test-exec} test/locals.lua && \
	${test-exec} test/constructs.lua && \
	${test-exec} test/bitbench.lua && \
	${test-exec} test/cjson-test.lua && \
	${test-exec} test/test_luazen.lua && \
	${test-exec} test/schema.lua && \
	test/integration_asymmetric_crypto.sh && \
	echo "----------------\nAll tests passed for SHARED binary build\n----------------"

check-static: test-exec := ${pwd}/src/zenroom-static -c ${pwd}/test/decode-test.conf
check-static: check-milagro
	@${test-exec} test/vararg.lua && \
	${test-exec} test/pm.lua && \
	${test-exec} test/nextvar.lua && \
	${test-exec} test/locals.lua && \
	${test-exec} test/constructs.lua && \
	${test-exec} test/bitbench.lua && \
	${test-exec} test/cjson-test.lua && \
	${test-exec} test/test_luazen.lua && \
	${test-exec} test/schema.lua && \
	test/integration_asymmetric_crypto.sh zenroom-static && \
	echo "----------------\nAll tests passed for STATIC binary build\n----------------"

check-js: test-exec := nodejs ${pwd}/test/zenroom_exec.js ${pwd}/src/zenroom.js
check-js:
	@${test-exec} test/vararg.lua && \
	${test-exec} test/nextvar.lua && \
	${test-exec} test/locals.lua && \
	${test-exec} test/constructs.lua && \
	${test-exec} test/bitbench.lua && \
	${test-exec} test/cjson-test.lua && \
	${test-exec} test/test_luazen.lua && \
	echo "----------------\nAll tests passed for JAVASCRIPT binary build\n----------------"

# TODO: complete js tests with schema and other lua extensions

check-debug: test-exec := valgrind ${pwd}/src/zenroom-shared -c ${pwd}/test/decode-test.conf
check-debug: check-milagro
	@${test-exec} test/vararg.lua && \
	${test-exec} test/pm.lua && \
	${test-exec} test/nextvar.lua && \
	${test-exec} test/locals.lua && \
	${test-exec} test/constructs.lua && \
	${test-exec} test/bitbench.lua && \
	${test-exec} test/cjson-test.lua && \
	${test-exec} test/test_luazen.lua && \
	${test-exec} test/schema.lua && \
	test/integration_asymmetric_crypto.sh && \
	echo "----------------\nAll tests passed for SHARED binary build\n----------------"

clean:
	rm -rf ${luasand}
	make clean -C ${pwd}/lib/lua53/src
	make clean -C ${pwd}/build/luazen
	make clean -C ${pwd}/lib/milagro-crypto-c && \
		rm -f ${pwd}/lib/milagro-crypto-c/CMakeCache.txt
	make clean -C src
	rm -f ${extras}/index.*

distclean:
	rm -rf ${musl}
