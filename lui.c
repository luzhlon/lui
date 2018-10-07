/* lui.c
 *
 * lua binding to libui (https://github.com/andlabs/libui)
 *
 * Gunnar Zötl <gz@tset.de>, 2016
 * Released under MIT/X11 license. See file LICENSE for details.
 */

const char *LUI_DEFAULT_WINDOW_TITLE = "(lui window)";
const int LUI_DEFAULT_WINDOW_WIDTH = 600;
const int LUI_DEFAULT_WINDOW_HEIGHT = 600;

/*** Module
 * Name: lui
 */

/*** Intro
 * lui is a cross platform gui library for lua, basically a souped up wrapper for libui (https://github.com/andlabs/libui).
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "ui.h"

#include "lua.h"
#include "lauxlib.h"

/* very raw approximations of some newer functions for lua 5.1 */
#if LUA_VERSION_NUM == 501
#define luaL_newlib(L,funcs) lua_newtable(L); luaL_register(L, NULL, funcs)
/* ignores nup, so can only be used for functions without shared upvalues */
#define luaL_setfuncs(L,funcs,x) luaL_register(L, NULL, funcs)
/* only equivalent of the uservalue is a table */
#define lua_setuservalue(L, idx) lua_setfenv(L, idx)
/* on lua 5.1, an empty table is returned if no setfenv() call preceded this.
 * On later luas, nil is returned when the uservalue was not set. */
#define lua_getuservalue(L, idx) lua_getfenv(L, idx)
#define LUA_OK 0
#endif

#define NOT_IMPLEMENTED return luaL_error(L, "Not implemented!");

static int lui_initialized = 0;

/* utility stuff  **********************************************************/

/* lui functions need to check this */
#define ensure_initialized() do { \
	if (!lui_initialized) { \
		luaL_error(L, "lui has not been initialized"); \
	} \
} while (0)

/* lui control methods need to check this */
#define ensure_valid(lobj) do { \
	if (lobj->object == 0) { \
		luaL_error(L, "control has already been destroyed"); \
	} \
} while (0)

typedef struct {
	void *object;
} lui_object;

#define LUI_OBJECT_REGISTRY "lui_object_registry"

#define lui_enumItem(N, V) lua_pushstring(L, (N)); lua_pushinteger(L, (V)); lua_settable(L, -3);

#ifdef DEBUG

#include <stdio.h>

#define DEBUGMSG(...) do { fprintf(stderr, __VA_ARGS__); fputs("\n", stderr); } while (0)

static const char* lui_debug_controlTostring(lua_State *L, int n)
{
	static char buf[256];
	void *obj = lua_touserdata(L, n);
	const char *type;
	if (luaL_getmetafield (L, n, "__name") != LUA_TNIL) {
		type = lua_tostring(L, -1);
		lua_pop(L, 1);
	} else {
		type = "lui_control";
	}
	snprintf(buf, 256, "%s: %p", type, ((lui_object*)obj)->object);
	return buf;
}

#else

#define DEBUGMSG(...)
#define lui_debug_controlTostring(L, n) "[lui_debug_controlTostring]"

#endif

#define lui_aux_istable(L, n) (lua_type((L), (n)) == LUA_TTABLE)

static int lui_aux_iscallable(lua_State *L, int pos)
{
	int type = lua_type(L, pos);
	if (type == LUA_TFUNCTION) {
		return 1;
	}
	int ctype = luaL_getmetafield(L, pos, "__call");
	if (ctype != LUA_TNIL) {
		lua_pop(L, 1);
	}
	return ctype == LUA_TFUNCTION;
}

/* pos ==0 or pos ==1 inserts at start, pos > len or pos < 0 inserts at end */
static void lui_aux_tinsert(lua_State *L, int tbl, int pos, int val)
{
	int len = lua_rawlen(L, tbl);
	if (pos == 0) {
		pos = 1;
	}
	if (pos >= 1 && pos <= len) {
		for (int cnt = len; cnt >= pos; --cnt) {
			lua_rawgeti(L, tbl, cnt);
			lua_rawseti(L, tbl, cnt + 1);
		}
	} else if (pos < 0 || pos > len) {
		pos = len + 1;
	}
	lua_pushvalue(L, val);
	lua_rawseti(L, tbl, pos);
}

/* only indices between 1 and len are considered */
static void lui_aux_tdelete(lua_State *L, int tbl, int pos)
{
	int len = lua_rawlen(L, tbl);
	if (pos < 1 || pos > len) {
		return;
	}
	if (pos < len) {
		for (int cnt = pos; cnt < len; ++cnt) {
			lua_rawgeti(L, tbl, cnt + 1);
			lua_rawseti(L, tbl, cnt);
		}
	}
	lua_pushnil(L);
	lua_rawseti(L, tbl, len);
}

static int lui_aux_getUservalue(lua_State *L, int pos, const char *name)
{
	if (lua_getuservalue(L, pos) != LUA_TNIL) {
		lua_getfield(L, -1, name);
		lua_copy(L, -1, -2);
		lua_pop(L, 1);
	}
	return lua_type(L, -1);
}

static int lui_aux_setUservalue(lua_State *L, int pos, const char *name, int vpos)
{
	int top = lua_gettop(L);
	if (vpos < 0) {
		vpos += 1 + top;
	}
	lua_getuservalue(L, pos);
	lua_pushvalue(L, vpos);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
	return 0;
}

static int lui_aux_clearUservalue(lua_State *L, int pos, const char *name)
{
	lua_getuservalue(L, pos);
	lua_pushnil(L);
	lua_setfield(L, -2, name);
	lua_pop(L, 1);
	return 0;
}

static int lui_aux_getRegistryTableField(lua_State *L, const char *tbl, const char *field)
{
	lua_getfield(L, LUA_REGISTRYINDEX, tbl);
	lua_getfield(L, -1, field);
	lua_copy(L, -1, -2);
	lua_pop(L, 1);
	return lua_type(L, -1);
}

static int lui_aux_getNumberOrValue(lua_State *L, int pos, const char *map)
{
	int isnum;
	int res = lua_tonumberx(L, pos, &isnum);
	if (!isnum) {
		const char *name = lua_tostring(L, pos);
		if (name) {
			if (lui_aux_getRegistryTableField(L, map, name) == LUA_TNUMBER) {
				res = lua_tonumber(L, -1);
			}
			lua_pop(L, 1);
		}
	}
	return res;
}

static const int lui_aux_pushNameOrValue(lua_State *L, int value, const char *map)
{
	if (lua_getfield(L, LUA_REGISTRYINDEX, map) != LUA_TTABLE) {
		lua_pop(L, 1);
		lua_pushinteger(L, value);
		return 1;
	}
	int t = lua_gettop(L);
	lua_pushnil(L);
	while (lua_next(L, t) != 0) {
		if (lua_tonumber(L, -1) == value) {
			lua_copy(L, -2, t);
			lua_pop(L, 2);
			return 1;
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 1);
	lua_pushinteger(L, value);
	return 1;
}

static const int lui_aux_setFieldsFromTable(lua_State *L, int obj, int tbl)
{
	lua_pushnil(L);
	while (lua_next(L, tbl) != 0) {
		lua_pushvalue(L, -2);
		lua_pushvalue(L, -2);
		lua_settable(L, obj);
		lua_pop(L, 1);
	}
	return 0;
}

/* generic lui_object handling  *********************************************/

/* lui_toObject
 *
 * return a pointer to the userdata in position pos of the stack, cast to
 * a lui_object*
 */
static lui_object* lui_toObject(lua_State *L, int pos)
{
	lui_object *lobj = (lui_object*) lua_touserdata(L, pos);
	return lobj;
}

static void* lui_throwWrongObjectError(lua_State *L, int pos)
{
	char buf[256];
	snprintf(buf, 256, "(expected lui object, got %s)", lua_typename(L, pos));
	luaL_argerror(L, pos, buf);
	return (void*)0;
}

/* lui_isObject
 * check if the data in position pos of the stack is a lui_object*, return
 * it if so, return NULL otherwise
 */
static lui_object* lui_isObject(lua_State *L, int pos)
{
	lui_object *lobj = (lui_object*) lua_touserdata(L, pos);
	if (!lobj || luaL_getmetafield(L, pos, "__name") == LUA_TNIL) {
		return 0;
	}
	const char *name = lua_tostring(L, -1);
	lua_pop(L, 1);
	if (strncmp(name, "lui_", 4) != 0) {
		return 0;
	}
	return lobj;
}
/* lui_checkObject
 *
 * check if the data in position pos of the stack is a lui_object*, return
 * it if so, fail otherwise
 */
static lui_object* lui_checkObject(lua_State *L, int pos)
{
	lui_object *lobj = (lui_object*) lui_isObject(L, pos);
	if (!lobj) {
		return lui_throwWrongObjectError(L, pos);
	}
	return lobj;
}

/* lui control registry handling
 */
static int lui_registerObject(lua_State *L, int pos)
{
	lui_object *lobj = lui_toObject(L, pos);
	lua_getfield(L, LUA_REGISTRYINDEX, LUI_OBJECT_REGISTRY);
	lua_pushlightuserdata(L, lobj->object);
	lua_pushvalue(L, pos);
	lua_settable(L, -3);
	lua_pop(L, 1);
	return 0;
}

static int lui_findObject(lua_State *L, const uiControl *control)
{
	lua_getfield(L, LUA_REGISTRYINDEX, LUI_OBJECT_REGISTRY);
	lua_pushlightuserdata(L, (uiControl*)control);
	lua_gettable(L, -2);
	lua_copy(L, -1, -2);
	lua_pop(L, 1);
	return lua_type(L, -1);
}

static lui_object* lui_pushObject(lua_State *L, const char *type, int needuv)
{
	ensure_initialized();
	lui_object *lobj = (lui_object*) lua_newuserdata(L, sizeof(lui_object));
	lobj->object = 0;
	luaL_getmetatable(L, type);
	lua_setmetatable(L, -2);
	if (needuv) {
		lua_newtable(L);
		lua_setuservalue(L, -2);
	}
	return lobj;
}

/* generic controls  ******************************************************/

/*** Object
 * Name: control
 * Generic UI control methods and properties. All controls have these. In
 * addition, all constructor methods can take a table of properties and
 * associated values as their last argument. If such a table is passed as
 * an argument, those properties are set after the control has been created.
 */

/* lui_control__gc
 *
 * __gc metamethod lui controls
 */
static int lui_control__gc(lua_State *L)
{
	lui_object *lobj = lui_toObject(L, 1);
	if (lobj->object) {
		if (lui_aux_getUservalue(L, 1, "parent") != LUA_TNIL) {
			DEBUGMSG("lui_control__gc (%s), has parent", lui_debug_controlTostring(L, 1));
			lui_aux_clearUservalue(L, 1, "parent");
			lobj->object = 0;
			lua_pop(L, 1);
			return 0;
		}
		lua_pop(L, 1);

		if (lui_aux_getUservalue(L, 1, "child") != LUA_TNIL) {
			DEBUGMSG("lui_control__gc (%s), has child", lui_debug_controlTostring(L, 1));
			lui_object *cobj = lui_toObject(L, 2);
			if (cobj->object) {
				lui_aux_clearUservalue(L, 2, "parent");
				cobj->object = 0;
			}
			uiControlDestroy(lobj->object);
			lobj->object = 0;
			lua_pop(L, 1);
			return 0;
		}
		lua_pop(L, 1);

		if (lui_aux_getUservalue(L, 1, "children") != LUA_TNIL) {
			DEBUGMSG("lui_control__gc (%s), has children", lui_debug_controlTostring(L, 1));
			int len = lua_rawlen(L, 2);
			for (int i = 1; i < len; ++i) {
				if (lua_rawgeti(L, 2, i) != LUA_TNIL) {
					lui_object *cobj = lui_toObject(L, 3);
					if (cobj->object) {
						lui_aux_clearUservalue(L, 3, "parent");
						cobj->object = 0;
					}
				}
				lua_pop(L, 1);
			}
			uiControlDestroy(lobj->object);
			lobj->object = 0;
			lua_pop(L, 1);
			return 0;
		}
		lua_pop(L, 1);
		
		DEBUGMSG("lui_control__gc (%s)", lui_debug_controlTostring(L, 1));
		uiControlDestroy(lobj->object);
		lobj->object = 0;
	}
	return 0;
}

/* lui_control__toString
 *
 * __tostring metamethod for lui controls
 */
static int lui_control__tostring(lua_State *L)
{
	void *obj = lua_touserdata(L, 1);
	const char *type;
	if (luaL_getmetafield (L, 1, "__name") != LUA_TNIL) {
		type = lua_tostring(L, -1);
		lua_pop(L, 1);
	} else {
		type = "lui_control";
	}
	lua_pushfstring(L, "%s: %p", type, ((lui_object*)obj)->object);
	return 1;
}

/*** Property
 * Object: control
 * Name: toplevel
 * true if the conrol is a toplevel control (i.e. window), false if not.
 * This is a read-only property.
 *** Property
 * Object: control
 * Name: visible
 * true if the control is / should be visible, false if not. Default is true.
 *** Property
 * Object: control
 * Name: enabled
 * true if the control is / should be enabled, false if not. Default is true.
 */
static int lui_control__index(lua_State *L)
{
	lui_object *lobj = lui_toObject(L, 1);
	const char *what = luaL_checkstring(L, 2);
	ensure_valid(lobj);

	if (strcmp(what, "toplevel") == 0) {
		lua_pushboolean(L, uiControlToplevel(lobj->object) != 0);
	} else if (strcmp(what, "visible") == 0) {
		lua_pushboolean(L, uiControlVisible(lobj->object) != 0);
	} else if (strcmp(what, "enabled") == 0) {
		lua_pushboolean(L, uiControlEnabled(lobj->object) != 0);
	} else if (luaL_getmetafield(L, 1, "__methods") != LUA_TNIL) {
		lua_pushvalue(L, 2);
		lua_gettable(L, -2);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

static int lui_control__newindex(lua_State *L)
{
	lui_object *lobj = lui_toObject(L, 1);
	const char *what = luaL_checkstring(L, 2);
	int val = lua_toboolean(L, 3);
	ensure_valid(lobj);

	if (strcmp(what, "visible") == 0) {
		if (val) {
			uiControlShow(lobj->object);
		} else {
			uiControlHide(lobj->object);
		}
	} else if (strcmp(what, "enabled") == 0) {
		if (val) {
			uiControlEnable(lobj->object);
		} else {
			uiControlDisable(lobj->object);
		}
	} else {
		lua_pushfstring(L, "attempt to set invalid field ('%s')", what);
		return lua_error(L);
	}
	return 1;
}

static void lui_controlSetParent(lua_State *L, int ctl, int parent)
{
	if (!lua_isnil(L, ctl)) {
		lui_aux_setUservalue(L, ctl, "parent", parent);
	}
}

static void lui_controlClearParent(lua_State *L, int ctl)
{
	if (!lua_isnil(L, ctl)) {
		lua_pushnil(L);
		lui_aux_setUservalue(L, ctl, "parent", lua_gettop(L));
		lua_pop(L, 1);
	}
}

static int lui_controlHasParent(lua_State *L, int ctl)
{
	int has = 0;
	if (lui_aux_getUservalue(L, ctl, "parent") != LUA_TNIL) {
		has = 1;
	}
	lua_pop(L, 1);
	return has;
}

static void lui_controlSetChild(lua_State *L, int ctl, int cld)
{
	if (lui_controlHasParent(L, cld)) {
		/*return*/ luaL_error(L, "child control already has a parent.");
	}
	if (lui_aux_getUservalue(L, ctl, "child") != LUA_TNIL) {
		lui_controlClearParent(L, lua_gettop(L));
	}
	lua_pop(L, 1);
	lui_aux_setUservalue(L, ctl, "child", cld);
	lui_controlSetParent(L, cld, ctl);
}

static void lui_controlInsertChild(lua_State *L, int ctl, int pos, int cld)
{
	if (lui_controlHasParent(L, cld)) {
		/*return*/ luaL_error(L, "child control already has a parent.");
	}
	if (lui_aux_getUservalue(L, ctl, "children") == LUA_TNIL) {
		lua_pop(L, 1);
		lua_newtable(L);
	}
	int tbl = lua_gettop(L);
	lui_aux_tinsert(L, tbl, pos, cld);
	lui_aux_setUservalue(L, ctl, "children", tbl);
	lui_controlSetParent(L, cld, ctl);
	lua_pop(L, 1);
}

static void lui_controlAppendChild(lua_State *L, int ctl, int cld)
{
	lui_controlInsertChild(L, ctl, -1, cld);
}

static void lui_controlDelChild(lua_State *L, int ctl, int pos)
{
	if (lui_aux_getUservalue(L, ctl, "children") != LUA_TNIL) {
		int tbl = lua_gettop(L);
		if (lua_rawgeti(L, tbl, pos) != LUA_TNIL) {
			lui_controlClearParent(L, lua_gettop(L));
		}
		lua_pop(L, 1);
		lui_aux_tdelete(L, tbl, pos);
	}
	lua_pop(L, 1);
}

static int lui_controlNchildren(lua_State *L, int ctl)
{
	int len = 0;
	if (lui_aux_getUservalue(L, ctl, "children") != LUA_TNIL) {
		len = lua_rawlen(L, -1);
	} else if (lui_aux_getUservalue(L, ctl, "child") != LUA_TNIL) {
		len = 1;
	}
	lua_pop(L, 1);
	return len;
}

/*** Method
 * Object: control
 * Name: numchildren
 * Signature: num = control:numchildren()
 * return the number of children a control has.
 */
static int lui_controlNumChildren(lua_State *L)
{
	(void) lui_checkObject(L, 1);
	lua_pushinteger(L, lui_controlNchildren(L, 1));
	return 1;
}

/*** Method
 * Object: control
 * Name: getchild
 * Signature: control = control:getchild(num)
 * return the num'th child of a control, or nil if there is no such child.
 */
static int lui_controlGetChild(lua_State *L)
{
	(void) lui_checkObject(L, 1);
	int which = luaL_checkinteger(L, 2);
	if (lui_aux_getUservalue(L, 1, "child") != LUA_TNIL) {
		if (which != 1) {
			lua_pop(L, 1);
			lua_pushnil(L);
		}
		return 1;
	}
	lua_pop(L, 1);
	if (lui_aux_getUservalue(L, 1, "children") != LUA_TNIL) {
		lua_rawgeti(L, lua_gettop(L), which);
		return 1;
	}
	lua_pop(L, 1);
	lua_pushnil(L);
	return 1;
}

/*** Method
 * Object: control
 * Name: getparent
 * Signature: control = control:getparent()
 * return the parent of a control or nil if it has no parent.
 */
static int lui_controlGetParent(lua_State *L)
{
	lui_object *lobj = lui_checkObject(L, 1);
	uiControl *ctl = uiControlParent(uiControl(lobj->object));
	lui_findObject(L, ctl);
	return 1;
}

/*** Method
 * Object: control
 * Name: type
 * Signature: control:type()
 * return the type name for a control
 */
static int lui_objectType(lua_State *L)
{
	lui_object *lobj = lui_checkObject(L, 1);
	if (lobj->object == 0) {
		lua_pushnil(L);
		lua_pushstring(L, "control has been destroyed");
		return 2;
	}
	int ok = luaL_getmetafield(L, 1, "__name");
	if (ok) {
		return 1;
	}
	lua_pushnil(L);
	lua_pushstring(L, "unknown control type");
	return 2;
}

static int lui_objectSetHandler(lua_State *L, const char *handler, int pos)
{
	if (!lua_isnoneornil(L, pos)) {
		luaL_argcheck(L, lui_aux_iscallable(L, pos), pos, "expected callable");
	}
	lui_aux_setUservalue(L, 1, handler, pos);
	return 0;
}

static int lui_objectGetHandler(lua_State *L, const char *handler)
{
	lui_aux_getUservalue(L, 1, handler);
	return 1;
}

/* beware: no stack hygenie. Do that in the concrete handler! */
static int lui_objectHandlerCallback(lua_State *L, const uiControl *control, const char *handler, int first, int narg, int nres)
{
	if (lui_findObject(L, control) == LUA_TNIL) {
		return 0;
	}
	int objidx = lua_gettop(L);
	int baseargs = 1;
	lui_aux_getUservalue(L, objidx, handler);
	if (!lui_aux_iscallable(L, -1)) {
		return 0;
	}
	if (luaL_getmetafield(L, -1, "__call") != LUA_TNIL) {
		lua_pushvalue(L, -2);
		baseargs += 1;
	}
	lua_pushvalue(L, objidx);
	int i;
	for (i = 0; i < narg; ++i) {
		lua_pushvalue(L, first + i);
	}
	lua_call(L, narg + baseargs, nres);

	return nres;
}

/* generic metamethods for uiControls */
static const luaL_Reg lui_control_meta[] = {
	{"__gc", lui_control__gc},
	{"__tostring", lui_control__tostring},
	{"__index", lui_control__index},
	{"__newindex", lui_control__newindex},
	{0, 0}
};

/* generic methods for uiControls */
static const luaL_Reg lui_control_methods[] = {
	{"type", lui_objectType},
	{"getparent", lui_controlGetParent},
	{"getchild", lui_controlGetChild},
	{"numchildren", lui_controlNumChildren},
	{0, 0}
};

/* utility object metamethods  ********************************************/

static int lui_utility__gc(lua_State *L)
{
	lui_object *lobj = lui_toObject(L, 1);
	if (lobj->object) {
		DEBUGMSG("lui_utility__gc (%s)", lui_debug_controlTostring(L, 1));
		free((void*)(lobj->object));
		lobj->object = 0;
	}
	return 0;
}

static int lui_utility__index(lua_State *L)
{
	(void) lui_checkObject(L, 1);

	if (luaL_getmetafield(L, 1, "__methods") != LUA_TNIL) {
		lua_pushvalue(L, 2);
		lua_gettable(L, -2);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

static int lui_utility__newindex(lua_State *L)
{
	const char *what = luaL_checkstring(L, 2);
	lua_pushfstring(L, "attempt to set invalid field ('%s')", what);
	return lua_error(L);
}

static const luaL_Reg lui_utility_meta[] = {
	{"__gc", lui_utility__gc},
	{"__tostring", lui_control__tostring},
	{"__index", lui_utility__index},
	{"__newindex", lui_utility__newindex},
	{0, 0}
};

/* helpers to register types for lui **************************************/

static void lui_add_control_type(lua_State *L, const char *name, const struct luaL_Reg *methods, const struct luaL_Reg *meta)
{
	luaL_newmetatable(L, name);
	luaL_setfuncs(L, lui_control_meta, 0);
	if (meta) {
		luaL_setfuncs(L, meta, 0);
	}

	/* add methods */
	lua_pushstring(L, "__methods");
	luaL_newlib(L, lui_control_methods);
	if (methods) {
		luaL_setfuncs(L, methods, 0);
	}
	lua_settable(L, -3);

	/* clean up stack */
	lua_pop(L, 1);
}

static void lui_add_utility_type(lua_State *L, const char *name, const struct luaL_Reg *methods, const struct luaL_Reg *meta)
{
	luaL_newmetatable(L, name);
	luaL_setfuncs(L, lui_utility_meta, 0);
	if (meta) {
		luaL_setfuncs(L, meta, 0);
	}

	if (methods) {
		lua_pushstring(L, "__methods");
		lua_newtable(L);
		luaL_setfuncs(L, methods, 0);
		lua_settable(L, -3);
	}

	/* clean up stack */
	lua_pop(L, 1);
}

/* color handling helper functions ****************************************/

static int lui_aux_pushRgbaAsTable(lua_State *L, double r, double g, double b, double a)
{
	lua_newtable(L);
	lua_pushnumber(L, r);
	lua_setfield(L, -2, "r");
	lua_pushnumber(L, g);
	lua_setfield(L, -2, "g");
	lua_pushnumber(L, b);
	lua_setfield(L, -2, "b");
	lua_pushnumber(L, a);
	lua_setfield(L, -2, "a");
	return 1;
}

static int lui_aux_rgbaFromTable(lua_State *L, int pos, double *r, double *g, double *b, double *a)
{
	if (pos < 0) {
		pos = lua_gettop(L) + 1 + pos;
	}
	lua_getfield(L, pos, "r");
	*r = luaL_checknumber(L, -1);
	lua_getfield(L, pos, "g");
	*g = luaL_checknumber(L, -1);
	lua_getfield(L, pos, "b");
	*b = luaL_checknumber(L, -1);
	lua_getfield(L, pos, "a");
	*a = luaL_optnumber(L, -1, 1);
	lua_pop(L, 4);
	return 1;
}	

/* include controls *******************************************************/

#include "container.inc.c"
#include "controls.inc.c"
#include "menu.inc.c"
#include "text.inc.c"
#include "draw.inc.c"
#include "area.inc.c"
#include "dialog.inc.c"
#include "image.inc.c"
#include "table.inc.c"

/* misc functions  *********************************************************/

/*** Function
 * Name: init
 * Signature: lui.init()
 * initialize lui, must be called before anything else.
 */
static int lui_init(lua_State *L)
{
	uiInitOptions o;
	memset(&o, 0, sizeof(o));
	const char *emsg = uiInit(&o);
	if (emsg) {
		lua_pushnil(L);
		lua_pushstring(L, emsg);
		uiFreeInitError(emsg);
		return 2;
	}
	lui_initialized = 1;
	lua_pushboolean(L, 1);
	return 1;
}

/*** Function
 * Name: finalize
 * Signature: lui.finalize()
 * finalize lui and clean up any used resources. May be omitted as lui is
 * finalized when the program terminates.
 */
static int lui_finalize(lua_State *L)
{
	if (lui_initialized) {
		DEBUGMSG("finalizing...");
		lui_initialized = 0;

		/* clean out control registry. The actual loop running through the
		 * registry must be repeated until there are no more valid objects
		 * left, as any of the objects in the registry may be a container
		 * holding other objects, that will have to be released before they
		 * can be collected.
		 */
		lua_pushstring(L, LUI_OBJECT_REGISTRY);
		lua_gettable(L, LUA_REGISTRYINDEX);
		int tbl = lua_gettop(L);
		lua_pushnil(L);
		int obidx = lua_gettop(L) + 1;
		while (lua_next(L, tbl) != 0) {
			lui_object *lobj = lui_isObject(L, -1);
			if (lobj && lobj->object) {
				DEBUGMSG("finalizing %s", lui_debug_controlTostring(L, -1));
				luaL_callmeta(L, obidx, "__gc");
			}
			lua_settop(L, obidx - 1);
		}
		lua_pop(L, 1);
		lua_pushstring(L, LUI_OBJECT_REGISTRY);
		lua_pushnil(L);
		lua_settable(L, LUA_REGISTRYINDEX);

		uiUninit();
	}
	return 0;
}

/*** Function
 * Name: main
 * Signature: lui.main()
 * enter the main event processing loop.
 */
static int lui_main(lua_State *L)
{
	uiMain();
	return 0;
}

/*** Function
 * Name: mainsteps
 * Signature: lui.mainsteps()
 * initialize your own main loop with lui.mainstep() instead of using
 * lui.main(). So, if you want to use your own main loop instead of
 * lui.main(), you do it like this:
 *
 * 	lui.mainsteps()
 * 	while lui.mainstep() do
 * 		-- stuff
 * 	end
 */
static int lui_mainSteps(lua_State *L)
{
	uiMainSteps();
	return 0;
}

/*** Function
 * Name: mainstep
 * Signature: running = lui.mainstep(wait = false)
 * perform one round of the main event processing loop. Returns true if the
 * program should continue to run, or false if it should quit (i.e. lui.quit()
 * was called).
 */
static int lui_mainStep(lua_State *L)
{
	int wait = lua_toboolean(L, 1);
	int cont = uiMainStep(wait);
	lua_pushboolean(L, cont);
	return 1;
}

/*** Function
 * Name: onnextidle
 * Signature: lui.onnextidle(function)
 * register a function to be called the next time when there are no events
 * pending. This function will only be called once.
 */
static void lui_onNextIdleCallback(void *data)
{
	DEBUGMSG("lui_onNextIdleCallback");
	lua_State *L = (lua_State*) data;
	lua_pushstring(L, LUI_OBJECT_REGISTRY);
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_pushstring(L, "lui_onnextidle");
	if (lua_gettable(L, -2) != LUA_TNIL) {
		lua_call(L, 0, 0);
	}
	lua_pop(L, 2);
}

static int lui_onNextIdle(lua_State *L)
{
	if (!lua_isnoneornil(L, 1)) {
		luaL_checktype(L, 1, LUA_TFUNCTION);
	}
	lua_pushstring(L, LUI_OBJECT_REGISTRY);
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_pushstring(L, "lui_onnextidle");
	lua_pushvalue(L, 1);
	lua_settable(L, -3);
	lua_pop(L, 1);
	uiQueueMain(lui_onNextIdleCallback, L);
	return 0;
}

/*** Function
 * Name: quit
 * Signature: lui.quit()
 * terminate the main event processing loop. After calling this, lui.main()
 * returns and lui.mainstep() returns false.
 */
static int lui_quit(lua_State *L)
{
	uiQuit();
	return 0;
}

/*** Function
 * Name: onshouldquit
 * Signature: lui.onshouldquit(function)
 * register a function to be called when the quit menu item (see above) is clicked.
 */
static int lui_onShouldQuit(lua_State *L)
{
	if (!lua_isnoneornil(L, 1)) {
		luaL_checktype(L, 1, LUA_TFUNCTION);
	}
	lua_pushstring(L, LUI_OBJECT_REGISTRY);
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_pushstring(L, "lui_onshouldquit");
	lua_pushvalue(L, 1);
	lua_settable(L, -3);
	lua_pop(L, 1);
	return 0;
}

static int lui_onShouldQuitCallback(void *data)
{
	DEBUGMSG("lui_onShouldQuitCallback");
	int ok = 1;
	lua_State *L = (lua_State*) data;
	lua_pushstring(L, LUI_OBJECT_REGISTRY);
	lua_gettable(L, LUA_REGISTRYINDEX);
	lua_pushstring(L, "lui_onshouldquit");
	if (lua_gettable(L, -2) != LUA_TNIL) {
		lua_call(L, 0, 1);
		ok = lua_toboolean(L, -1);
		lua_pop(L, 1);
	}
	lua_pop(L, 2);
	return ok;
}

static int lui_fileDialog(lua_State *L, uiWindow *parent, char* (dlgfn)(uiWindow*))
{
	char *file = dlgfn(parent);
	if (file) {
		lua_pushstring(L, file);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

/*** Function
 * Name: openfile
 * Signature: filename = lui.openfile(window)
 * open a system file open dialog. Returns the path and name of the file to
 * open, or nil if the dialog was cancelled.
 */
static int lui_openFile(lua_State *L)
{
	lui_object *lobj = lui_checkWindow(L, 1);
	return lui_fileDialog(L, uiWindow(lobj->object), uiOpenFile);
}

/*** Function
 * Name: savefile
 * Signature: filename = lui.savefile(window)
 * open a system file save dialog. Returns the path and name of the file to
 * save to, or nil if the dialog was cancelled.
 */
static int lui_saveFile(lua_State *L)
{
	lui_object *lobj = lui_checkWindow(L, 1);
	return lui_fileDialog(L, uiWindow(lobj->object), uiSaveFile);
}

/*** Function
 * Name: msgbox
 * Signature: lui.msgbox(window, title, text)
 * open a system message box.
 */
static int lui_msgBox(lua_State *L)
{
	lui_object *lobj = lui_checkWindow(L, 1);
	const char *title = luaL_checkstring(L, 2);
	const char *text = luaL_checkstring(L, 3);
	uiMsgBox(uiWindow(lobj->object), title, text);
	return 0;
}

/*** Function
 * Name: errorbox
 * Signature: lui.errorbox(window, title, text)
 * open a system error message box.
 */
static int lui_errorBox(lua_State *L)
{
	lui_object *lobj = lui_checkWindow(L, 1);
	const char *title = luaL_checkstring(L, 2);
	const char *text = luaL_checkstring(L, 3);
	uiMsgBoxError(uiWindow(lobj->object), title, text);
	return 0;
}

/* module function list table
 */
static const struct luaL_Reg lui_funcs [] ={
	{"init", lui_init},
	{"finalize", lui_finalize},
	{"main", lui_main},
	{"mainsteps", lui_mainSteps},
	{"mainstep", lui_mainStep},
	{"onnextidle", lui_onNextIdle},
	{"quit", lui_quit},
	{"onshouldquit", lui_onShouldQuit},
	{"openfile", lui_openFile},
	{"savefile", lui_saveFile},
	{"msgbox", lui_msgBox},
	{"errorbox", lui_errorBox},
	{0, 0}
};

/* luaopen_lui
 * 
 * open and initialize this library
 */
int luaopen_lui(lua_State *L)
{
	/* exported function table. Create a metatable with a __gc field in order
	 * to ensure that on lua closedown lui_finalize() gets called.
	 */
	luaL_newlib(L, lui_funcs);
	lua_newtable(L);
	lua_pushstring(L, "__gc");
	lua_pushcfunction(L, lui_finalize);
	lua_settable(L, -3);
	lua_setmetatable(L, -2);

	lui_init_container(L);
	lui_init_controls(L);
	lui_init_menu(L);
	lui_init_text(L);
	lui_init_draw(L);
	lui_init_area(L);
	lui_init_dialog(L);
	lui_init_image(L);
	lui_init_table(L);

	/* create control registry */
	lua_newtable(L);
	lua_newtable(L);
	lua_pushstring(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
	lua_setfield(L, LUA_REGISTRYINDEX, LUI_OBJECT_REGISTRY);

	/* register global onShouldQuit handler */
	uiOnShouldQuit(lui_onShouldQuitCallback, L);

	return 1;
}
