/* luad.h
 *
 * some lua debugging helpers
 *
 * Gunnar Zötl <gz@tset.de>, 2017
 * Released under MIT/X11 license. See file LICENSE for details.
 */

#ifndef luad_h
#define luad_h

#define LUAD_TOSTRING_BUFFERSIZE 256

const char *luad_tostring(lua_State *L, int idx);
void luad_print(lua_State *L, int idx);
void luad_dump_stack(lua_State *L);
void luad_dump_table(lua_State *L, int idx);
void luad_dump_registry(lua_State *L);
void luad_dump_uservalue(lua_State *L, int idx);
void luad_dump_traceback(lua_State *L);

#endif // luad_h
