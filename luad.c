/* luad.c
 *
 * some lua debugging helpers
 *
 * Gunnar Zötl <gz@tset.de>, 2017
 * Released under MIT/X11 license. See file LICENSE for details.
 */

#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include "luad.h"

const char *luad_tostring(lua_State *L, int idx)
{
	static char buffer[LUAD_TOSTRING_BUFFERSIZE];
	const char *typename = NULL;
	int type = lua_type(L, idx);
	if (luaL_getmetafield(L, idx, "__name")) {
		typename = lua_tostring(L, -1);
		lua_pop(L, 1);
	} else {
		typename = lua_typename(L, type);
	}

	switch (type) {
		case LUA_TNIL:
			snprintf(buffer, LUAD_TOSTRING_BUFFERSIZE, "%s: %s", typename, "(nil)");
			break;
	    case LUA_TSTRING:
	        snprintf(buffer, LUAD_TOSTRING_BUFFERSIZE, "%s: '%s'", typename, lua_tostring(L, idx));
	        break;
	    case LUA_TBOOLEAN:
	        snprintf(buffer, LUAD_TOSTRING_BUFFERSIZE, "%s: %s", typename, (lua_toboolean(L, idx) ? "true" : "false"));
	        break;
	    case LUA_TNUMBER:
	    	#if LUA_VERSION_NUM >= 503
	    	if (lua_isinteger(L, idx)) {
	    		snprintf(buffer, LUAD_TOSTRING_BUFFERSIZE, "%s: (int) %lld", typename, lua_tointeger(L, idx));
	    	} else {
	    	#else
	    	{
	    	#endif
	        	snprintf(buffer, LUAD_TOSTRING_BUFFERSIZE, "%s: (flt) %f", typename, lua_tonumber(L, idx));
	        }
	        break;
	    default:
	        snprintf(buffer, LUAD_TOSTRING_BUFFERSIZE, "%s: %p", typename, lua_topointer(L, idx));
	        break;
	}
	return buffer;
}

void luad_print(lua_State *L, int idx)
{
	fprintf(stderr, "%s\n", luad_tostring(L, idx));
}

void luad_dump_stack(lua_State *L)
{
    int i;
    int top = lua_gettop(L);
    fprintf(stderr, "Lua Stack:\n");
    for (i = 1; i <= top; i++) {
    	fprintf(stderr, "%5d \t%s\n", i, luad_tostring(L, i));
    }
}

void luad_dump_table(lua_State *L, int idx)
{
	lua_pushnil(L);  /* first key */
	while (lua_next(L, idx) != 0) {
		fprintf(stderr, "%s \t: ", luad_tostring(L, -2));
		fprintf(stderr, "%s\n", luad_tostring(L, -1));
		lua_pop(L, 1);
	}
}

void luad_dump_registry(lua_State *L)
{
	fprintf(stderr, "Lua Registry:\n");
	luad_dump_table(L, LUA_REGISTRYINDEX);
}

void luad_dump_uservalue(lua_State *L, int idx)
{
	#if LUA_VERSION_NUM == 501
	lua_getfenv(L, idx);
	#else
	lua_getuservalue(L, idx);
	#endif
	printf("Uservalue for %s:\n", luad_tostring(L, idx));
	luad_dump_table(L, lua_gettop(L));
	lua_pop(L, 1);
}

void luad_dump_traceback(lua_State *L)
{
	luaL_traceback(L, L, 0, 0);
	puts(lua_tostring(L, -1));
	lua_pop(L, 1);
}

