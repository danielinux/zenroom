/*  Zenroom (DECODE project)
 *
 *  (c) Copyright 2017-2018 Dyne.org foundation
 *  designed, written and maintained by Denis Roio <jaromil@dyne.org>
 *
 * This source code is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Public License as published
 * by the Free Software Foundation; either version 3 of the License,
 * or (at your option) any later version.
 *
 * This source code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * Please refer to the GNU Public License for more details.
 *
 * You should have received a copy of the GNU Public License along with
 * this source code; if not, write to:
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <stdio.h>
#include <errno.h>
#include <jutils.h>

#include <lauxlib.h>

#include <zenroom.h>
#include <zen_error.h>

// passes the string to be printed through the 'tosting' function
// inside lua, taking care of some sanitization and conversions
static const char *lua_print_format(lua_State *L,
                                    int pos, size_t *len) {
	const char *s;
	lua_pushvalue(L, -1);  /* function to be called */
	lua_pushvalue(L, pos);   /* value to print */
	lua_call(L, 1, 1);
	s = lua_tolstring(L, -1, len);  /* get result */
	if (s == NULL)
		luaL_error(L, LUA_QL("tostring") " must return a string to "
		           LUA_QL("print"));
	return s;
}

// retrieves output buffer if configured in _Z and append to that the
// output without exceeding its length. Return 1 if output buffer was
// configured so calling function can decide if to proceed with other
// prints (stdout) or not
static int lua_print_tobuffer(lua_State *L) {
	lua_getglobal(L, "_Z");
	zenroom_t *Z = lua_touserdata(L, -1);
	lua_pop(L, 1);
	SAFE(Z);
	if(Z->stdout_buf && Z->stdout_pos < Z->stdout_len) {
		int i;
		int n = lua_gettop(L);  /* number of arguments */
		char *out = (char*)Z->stdout_buf;
		size_t len;
		lua_getglobal(L, "tostring");
		for (i=1; i<=n; i++) {
			const char *s = lua_print_format(L, i, &len);
			if(i>1) { out[Z->stdout_pos]='\t'; Z->stdout_pos++; }
			snprintf(out+Z->stdout_pos,
			         Z->stdout_len - Z->stdout_pos,
			         "%s", s);
			Z->stdout_pos+=len;
			lua_pop(L, 1);
		}
		return 1;
	}
	return 0;
}

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

static int zen_print (lua_State *L) {
	if( lua_print_tobuffer(L) ) return 0;

	char out[MAX_STRING];
	size_t pos = 0;
	size_t len = 0;
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s = lua_print_format(L, i, &len);
		if (i>1) { out[pos]='\t'; pos++; }
		snprintf(out+pos,MAX_STRING-pos,"%s",s);
		pos+=len;
		lua_pop(L, 1);  /* pop result */
	}
	EM_ASM_({Module.print(UTF8ToString($0))}, out);
	return 0;
}

static int zen_error (lua_State *L) {
	if( lua_print_tobuffer(L) ) return 0;

	char out[MAX_STRING];
	size_t pos = 0;
	size_t len = 0;
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	out[0] = '['; out[1] = '!';	out[2] = ']'; out[3] = ' ';	pos = 4;
	for (i=1; i<=n; i++) {
		const char *s = lua_print_format(L, i, &len);
		if (i>1) { out[pos]='\t'; pos++; }
		snprintf(out+pos,MAX_STRING-pos,"%s",s);
		pos+=len;
		lua_pop(L, 1);  /* pop result */
	}
	EM_ASM_({Module.print(UTF8ToString($0))}, out);
	return 0;
}

static int zen_iowrite (lua_State *L) {
	char out[MAX_STRING];
	size_t pos = 0;
	int nargs = lua_gettop(L) +1;
	int arg = 0;
	for (; nargs--; arg++) {
		size_t len;
		const char *s = lua_tolstring(L, arg, &len);
		if (arg>1) { out[pos]='\t'; pos++; }
		snprintf(out+pos,MAX_STRING-pos,"%s",s);
		pos+=len;
	}
	EM_ASM_({Module.print(UTF8ToString($0))}, out);
	lua_pushboolean(L, 1);
	return 1;
}


#else


static int zen_print (lua_State *L) {
	if( lua_print_tobuffer(L) ) return 0;

	int status = 1;
	size_t len = 0;
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	for (i=1; i<=n; i++) {
		const char *s = lua_print_format(L, i, &len);
		if(i>1) fwrite("\t",sizeof(char),1,stdout);
		status = status &&
			(fwrite(s, sizeof(char), len, stdout) == len);
		lua_pop(L, 1);  /* pop result */
	}
	fwrite("\n",sizeof(char),1,stdout);
	fflush(stdout);
	return 0;
}

static int zen_error (lua_State *L) {
	if( lua_print_tobuffer(L) ) return 0;

	int status = 1;
	size_t len = 0;
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	lua_getglobal(L, "tostring");
	fwrite("[!] ",sizeof(char),4,stderr);
	for (i=1; i<=n; i++) {
		const char *s = lua_print_format(L, i, &len);
		if(i>1) fwrite("\t",sizeof(char),1,stderr);
		status = status &&
			(fwrite(s, sizeof(char), len, stderr) == len);
		lua_pop(L, 1);  /* pop result */
	}
	fwrite("\n",sizeof(char),1,stderr);
	fflush(stderr);
	return 0;
}

static int zen_iowrite (lua_State *L) {
	int nargs = lua_gettop(L) +1;
	int status = 1;
	int arg = 0;
	FILE *out = stdout;
	for (; nargs--; arg++) {
		if (lua_type(L, arg) == LUA_TNUMBER) {
			/* optimization: could be done exactly as for strings */
			status = status &&
				fprintf(out, LUA_NUMBER_FMT, lua_tonumber(L, arg)) > 0;
		} else {
			size_t l;
			const char *s = lua_tolstring(L, arg, &l);
			status = status && (fwrite(s, sizeof(char), l, out) == l);
		}
	}
	if (!status) {
		lua_pushnil(L);
		lua_pushfstring(L, "%s", strerror(errno));
		lua_pushinteger(L, errno);
		return 3;
	}
	lua_pushboolean(L, 1);
	return 1;
}

#endif

void zen_add_io(lua_State *L) {
	// override print() and io.write()
	static const struct luaL_Reg custom_print [] =
		{ {"print", zen_print},
		  {"error", zen_error},
		  {NULL, NULL} };
	lua_getglobal(L, "_G");
	luaL_setfuncs(L, custom_print, 0);  // for Lua versions 5.2 or greater
	lua_pop(L, 1);

	static const struct luaL_Reg custom_iowrite [] =
		{ {"write", zen_iowrite}, {NULL, NULL} };
	lua_getglobal(L, "io");
	luaL_setfuncs(L, custom_iowrite, 0);
	lua_pop(L, 1);

}
