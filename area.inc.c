/* lui.c
 *
 * lua binding to libui (https://github.com/andlabs/libui)
 *
 * Gunnar Zötl <gz@tset.de>, 2016
 * Released under MIT/X11 license. See file LICENSE for details.
 *
 * This file is included by lui.c
 */

#ifdef uiAreaHandler
#error "uiAreaHandler() is defined now"
#endif
#define uiAreaHandler(this) ((uiAreaHandler*)(this))

/* area control  ***********************************************************/

/*** Object
 * Name: area
 * a rendering area for graphics or extended text rendering. Note that all
 * handlers are currently experiments, their arguments may change.
 */
#define LUI_AREA "lui_area"
#define lui_pushArea(L) lui_pushObject(L, LUI_AREA, 1)
#define lui_checkArea(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_AREA))

/*** Property
 * Object: area
 * Name: ondraw
 * a function <code>ondraw(area, context, x, y, w, h, areaw, areah)</code> that is
 * called when the area needs redrawing. area is the area this handler is
 * called for. context is a draw.context object for this area. x, y, w, h are
 * the position and size of the rectangle within the area to redraw. areaw,
 * areah are the full width and height of the area.
 *** Property
 * Object: area
 * Name: onmouse
 * a function <code>onmouse(area, which, button, x, y, count, modifiers, areaw, areah)</code>
 * that is called when there is mouse action in the area. which is the type
 * of action that occurred, and can be "down", "up", "move", "enter" and "leave".
 * The arguments after which are empty for the events "enter" and leave". For
 * the other events: button is the number of th button that was pressed. It
 * is 0 for the "move" event. x, y is the position within the area, where the
 * event occurred. count is a counter for mouse clicks. It can be used to
 * determine whether a click is a double click. modifiers are the modifier
 * keys. areaw, areah are the full width and height of the area.
 *** Property
 * Object: area
 * Name: onkey
 * a function <code>bool onkey(area, key, extkey, modifiers, up)</code>
 * that is called when there is a key event for the area. key is the code
 * of the key pressed, if it is none of the special keys. extkey is the name
 * of the key if it is a special key. modifiers is an or'ed mask of the
 * currently active modifiers, the names of which can be found in
 * lui.enum.keymod. So, in order to determine of a modifier is active, you
 * check for it using for example <code>modifier | lui.enum.keymod.shift</code>.
 *** Property_undocumented
 * Object: area
 * Name: ondragbroken
 * a function <code>ondragbroken()</code>
 * no idea what this is for
 */
static int lui_area__index(lua_State *L)
{
	(void) lui_checkArea(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "ondraw") == 0) {
		lui_objectGetHandler(L, "ondraw");
	} else if (strcmp(what, "onmouse") == 0) {
		lui_objectGetHandler(L, "onmouse");
	} else if (strcmp(what, "onkey") == 0) {
		lui_objectGetHandler(L, "onkey");
	} else if (strcmp(what, "ondragbroken") == 0) {
		lui_objectGetHandler(L, "ondragbroken");
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_area__newindex(lua_State *L)
{
	(void) lui_checkArea(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "ondraw") == 0) {
		lui_objectSetHandler(L, "ondraw", 3);
	} else if (strcmp(what, "onmouse") == 0) {
		lui_objectSetHandler(L, "onmouse", 3);
	} else if (strcmp(what, "onkey") == 0) {
		lui_objectSetHandler(L, "onkey", 3);
	} else if (strcmp(what, "ondragbroken") == 0) {
		lui_objectSetHandler(L, "ondragbroken", 3);
	} else {
		return lui_control__newindex(L);
	}
	return 0;
}

typedef struct {
	uiAreaHandler H;
	lua_State *L;
} lui_areaHandler;
#define lui_areaHandler(this) ((lui_areaHandler *)(this))

static void lui_areaDrawCallback(uiAreaHandler *ah, uiArea *area, uiAreaDrawParams *params)
{
	lua_State *L = lui_areaHandler(ah)->L;
	int top = lua_gettop(L);
	lui_wrapDrawcontext(L, params->Context);
	lua_pushnumber(L, params->ClipX);
	lua_pushnumber(L, params->ClipY);
	lua_pushnumber(L, params->ClipWidth);
	lua_pushnumber(L, params->ClipHeight);
	lua_pushnumber(L, params->AreaWidth);
	lua_pushnumber(L, params->AreaHeight);
	lui_objectHandlerCallback(L, uiControl(area), "ondraw", top + 1, 7, 0);
	lua_settop(L, top);
}

static void lui_areaMouseEventCallback(uiAreaHandler *ah, uiArea *area, uiAreaMouseEvent *evt)
{
	lua_State *L = lui_areaHandler(ah)->L;
	int top = lua_gettop(L);
	if (evt->Down) {
		lua_pushstring(L, "down");
		lua_pushinteger(L, evt->Down);
	} else if (evt->Up) {
		lua_pushstring(L, "up");
		lua_pushinteger(L, evt->Up);
	} else {
		lua_pushstring(L, "move");
		lua_pushinteger(L, 0);
	}
	lua_pushnumber(L, evt->X);
	lua_pushnumber(L, evt->Y);
	lua_pushinteger(L, evt->Count);
	lua_pushinteger(L, evt->Modifiers);
	lua_pushnumber(L, evt->AreaWidth);
	lua_pushnumber(L, evt->AreaHeight);
	lui_objectHandlerCallback(L, uiControl(area), "onmouse", top + 1, 8, 0);
	lua_settop(L, top);
}

static void lui_areaMouseCrossedCallback(uiAreaHandler *ah, uiArea *area, int left)
{
	lua_State *L = lui_areaHandler(ah)->L;
	int top = lua_gettop(L);
	lua_pushstring(L, left ? "leave" : "enter");
	lui_objectHandlerCallback(L, uiControl(area), "onmouse", top + 1, 1, 0);
	lua_settop(L, top);
}

// what is this for???
static void lui_areaDragBrokenCallback(uiAreaHandler *ah, uiArea *area)
{
	lua_State *L = lui_areaHandler(ah)->L;
	int top = lua_gettop(L);
	lui_objectHandlerCallback(L, uiControl(area), "ondragbroken", top + 1, 0, 0);
	lua_settop(L, top);
}

static int lui_areaPushExtKeyName(lua_State *L, int ext, int mod)
{
	if (ext) {
		lui_aux_pushNameOrValue(L, ext, "lui_enumkeyext");
	} else if (mod) {
		lui_aux_pushNameOrValue(L, mod, "lui_enumkeymod");
	}
	return 1;
}

static int lui_areaKeyEventCallback(uiAreaHandler *ah, uiArea *area, uiAreaKeyEvent *evt)
{
	lua_State *L = lui_areaHandler(ah)->L;
	int top = lua_gettop(L);
	if (evt->Key > 0) {
		lua_pushinteger(L, evt->Key);
	} else {
		lua_pushnil(L);
	}
	if (evt->ExtKey > 0 || evt->Modifier > 0) {
		lui_areaPushExtKeyName(L, evt->ExtKey, evt->Modifier);
	} else {
		lua_pushnil(L);
	}
	if (evt->Modifiers > 0) {
		lua_pushinteger(L, evt->Modifiers);
	} else {
		lua_pushnil(L);
	}
	lua_pushboolean(L, evt->Up);
	lui_objectHandlerCallback(L, uiControl(area), "onkey", top + 1, 4, 1);
	int ok = lua_toboolean(L, -1);
	lua_settop(L, top);
	return ok;
}

static int lui_makeAreaKeyModifierEnum(lua_State *L)
{
	lua_newtable(L);
	lui_enumItem("ctrl", uiModifierCtrl);
	lui_enumItem("alt", uiModifierAlt);
	lui_enumItem("shift", uiModifierShift);
	lui_enumItem("super", uiModifierSuper);
	return 1;
}

static int lui_makeAreaKeyExtEnum(lua_State *L)
{
	lua_newtable(L);
	lui_enumItem("escape", uiExtKeyEscape);
	lui_enumItem("insert", uiExtKeyInsert);
	lui_enumItem("delete", uiExtKeyDelete);
	lui_enumItem("home", uiExtKeyHome);
	lui_enumItem("end", uiExtKeyEnd);
	lui_enumItem("pageup", uiExtKeyPageUp);
	lui_enumItem("pagedown", uiExtKeyPageDown);
	lui_enumItem("up", uiExtKeyUp);
	lui_enumItem("down", uiExtKeyDown);
	lui_enumItem("left", uiExtKeyLeft);
	lui_enumItem("right", uiExtKeyRight);
	lui_enumItem("f1", uiExtKeyF1);
	lui_enumItem("f2", uiExtKeyF2);
	lui_enumItem("f3", uiExtKeyF3);
	lui_enumItem("f4", uiExtKeyF4);
	lui_enumItem("f5", uiExtKeyF5);
	lui_enumItem("f6", uiExtKeyF6);
	lui_enumItem("f7", uiExtKeyF7);
	lui_enumItem("f8", uiExtKeyF8);
	lui_enumItem("f9", uiExtKeyF9);
	lui_enumItem("f10", uiExtKeyF10);
	lui_enumItem("f11", uiExtKeyF11);
	lui_enumItem("f12", uiExtKeyF12);
	lui_enumItem("n0", uiExtKeyN0);
	lui_enumItem("n1", uiExtKeyN1);
	lui_enumItem("n2", uiExtKeyN2);
	lui_enumItem("n3", uiExtKeyN3);
	lui_enumItem("n4", uiExtKeyN4);
	lui_enumItem("n5", uiExtKeyN5);
	lui_enumItem("n6", uiExtKeyN6);
	lui_enumItem("n7", uiExtKeyN7);
	lui_enumItem("n8", uiExtKeyN8);
	lui_enumItem("n9", uiExtKeyN9);
	lui_enumItem("ndot", uiExtKeyNDot);
	lui_enumItem("nenter", uiExtKeyNEnter);
	lui_enumItem("nadd", uiExtKeyNAdd);
	lui_enumItem("nsubtract", uiExtKeyNSubtract);
	lui_enumItem("nmultiply", uiExtKeyNMultiply);
	lui_enumItem("ndivide", uiExtKeyNDivide);
	return 1;
};

static int luiMakeWindowEdgeEnum(lua_State *L)
{
	lua_newtable(L);
	lui_enumItem("left", uiWindowResizeEdgeLeft);
	lui_enumItem("top", uiWindowResizeEdgeTop);
	lui_enumItem("right", uiWindowResizeEdgeRight);
	lui_enumItem("bottom", uiWindowResizeEdgeBottom);
	lui_enumItem("topleft", uiWindowResizeEdgeTopLeft);
	lui_enumItem("topright", uiWindowResizeEdgeTopRight);
	lui_enumItem("bottomleft", uiWindowResizeEdgeBottomLeft);
	lui_enumItem("bottomright", uiWindowResizeEdgeBottomRight);
	return 1;
}

/* L will be filled when the commonAreaHandler is first used. */
static lui_areaHandler lui_commonAreaHandler = {
	.H.Draw = lui_areaDrawCallback,
	.H.MouseEvent = lui_areaMouseEventCallback,
	.H.MouseCrossed = lui_areaMouseCrossedCallback,
	.H.DragBroken = lui_areaDragBrokenCallback,
	.H.KeyEvent = lui_areaKeyEventCallback,
	.L = 0
};

/*** Method
 * Object: area
 * Name: setsize
 * Signature: area:setsize(width, height)
 * change the size of an area.
 */
static int lui_areaSetSize(lua_State *L)
{
	lui_object *lobj = lui_checkArea(L, 1);
	int width = luaL_checkinteger(L, 2);
	int height = luaL_checkinteger(L, 3);
	uiAreaSetSize(uiArea(lobj->object), width, height);
	return 0;
}

/*** Method
 * Object: area
 * Name: forceredraw
 * Signature: area:forceredraw()
 * force a redraw of the entire area.
 */
static int lui_areaForceRedraw(lua_State *L)
{
	lui_object *lobj = lui_checkArea(L, 1);
	uiAreaQueueRedrawAll(uiArea(lobj->object));
	return 0;
}

/*** Method
 * Object: area
 * Name: scrollto
 * Signature: area:scrollto(x, y, w, h)
 * make x, y, w, h the currently visible portion of the area.
 */
static int lui_areaScrollTo(lua_State *L)
{
	lui_object *lobj = lui_checkArea(L, 1);
	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	double width = luaL_checknumber(L, 4);
	double height = luaL_checknumber(L, 5);
	uiAreaScrollTo(uiArea(lobj->object), x, y, width, height);
	return 0;
}

/*** Method
 * Object: area
 * Name: beginuserwindowmove
 * Signature: area:beginuserwindowmove()
 * only usable within mouse events
 */
static int lui_areaBeginUserWindowMove(lua_State *L)
{
	lui_object *lobj = lui_checkArea(L, 1);
	uiAreaBeginUserWindowMove(uiArea(lobj->object));
	return 0;
}

/*** Method
 * Object: area
 * Name: beginuserwindowresize
 * Signature: area:beginuserwindowresize(edge)
 * only usable within mouse events. An enumeration of the edges can be found
 * in lui.enum.edge, or you may enter the name directly.
 */
static int lui_areaBeginUserWindowResize(lua_State *L)
{
	lui_object *lobj = lui_checkArea(L, 1);
	int edge = lui_aux_getNumberOrValue(L, 2, "lui_enumwinedge");
	uiAreaBeginUserWindowResize(uiArea(lobj->object), edge);
	return 0;
}


/*** Constructor
 * Object: area
 * Name: area
 * Signature: area = lui.area(width = 0, height = 0, properties = nil)
 * create a new area. Setting width and height to a value other than 0 makes
 * the area a scrolling area.
 */
static int lui_newArea(lua_State *L)
{
	lui_commonAreaHandler.L = L;
	int width = luaL_optinteger(L, 1, 0);
	int height = luaL_optinteger(L, 2, 0);
	int hastable = lui_aux_istable(L, 3);

	lui_object *lobj = lui_pushArea(L);
	if (width == 0 && height == 0) {
		lobj->object = uiNewArea(uiAreaHandler(&lui_commonAreaHandler));
	} else {
		lobj->object = uiNewScrollingArea(uiAreaHandler(&lui_commonAreaHandler), width, height);
	}
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 3); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for areas */
static const luaL_Reg lui_area_meta[] = {
	{"__index", lui_area__index},
	{"__newindex", lui_area__newindex},
	{0, 0}
};

/* methods for areas */
static const struct luaL_Reg lui_area_methods [] = {
	{"setsize", lui_areaSetSize},
	{"forceredraw", lui_areaForceRedraw},
	{"scrollto", lui_areaScrollTo},
	{"beginuserwindowmove", lui_areaBeginUserWindowMove},
	{"beginuserwindowresize", lui_areaBeginUserWindowResize},
	{0, 0}
};

/* fontbutton control  ****************************************************/

/*** Object
 * Name: fontbutton
 * a button to open a font selector.
 */
#define LUI_FONTBUTTON "lui_fontbutton"
#define lui_pushFontbutton(L) lui_pushObject(L, LUI_FONTBUTTON, 1)
#define lui_checkFontbutton(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_FONTBUTTON))

/*** Property
 * Object: fontbutton
 * Name: font
 * The font that was last selected via the font selector, or nil if none has
 * been selected yet. This is a read-only property.
 *** Property
 * Object: fontbutton
 * Name: onchanged
 * a function <code>onchanged(fontbutton)</code> that is called when a new
 * font was selected.
 */
static int lui_fontbutton__index(lua_State *L)
{
	lui_object *lobj = lui_checkFontbutton(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "font") == 0) {
		uiFontDescriptor *fnt = malloc(sizeof(uiFontDescriptor));
		uiFontButtonFont(uiFontButton(lobj->object), fnt);
		if (fnt->Family) {
			lui_wrapTextFont(L, fnt);
		} else {
			free(fnt);
			lua_pushnil(L);
		}
	} else if (strcmp(what, "onchanged") == 0) {
		lui_objectGetHandler(L, "onchanged");
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_fontbutton__newindex(lua_State *L)
{
	(void) lui_checkFontbutton(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "onchanged") == 0) {
		lui_objectSetHandler(L, "onchanged", 3);
	} else {
		return lui_control__newindex(L);
	}
	return 0;
}

static void lui_fontbuttonOnChangedCallback(uiFontButton *btn, void *data)
{
	lua_State *L = (lua_State*) data;
	int top = lua_gettop(L);
	lui_objectHandlerCallback(L, uiControl(btn), "onchanged", top, 0, 0);
	lua_settop(L, top);
}

/*** Constructor
 * Object: fontbutton
 * Name: fontbutton
 * Signature: fontbutton = lui.fontbutton(properties = nil)
 * create a new fontbutton.
 */
static int lui_newFontButton(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);

	lui_object *lobj = lui_pushFontbutton(L);
	lobj->object = uiNewFontButton();
	uiFontButtonOnChanged(uiFontButton(lobj->object), lui_fontbuttonOnChangedCallback, L);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for fontbuttons */
static const luaL_Reg lui_fontbutton_meta[] = {
	{"__index", lui_fontbutton__index},
	{"__newindex", lui_fontbutton__newindex},
	{0, 0}
};

/* colorbutton control  ***************************************************/

/*** Object
 * Name: colorbutton
 * a button to open a color selector.
 */
#define LUI_COLORBUTTON "lui_colorbutton"
#define lui_pushColorbutton(L) lui_pushObject(L, LUI_COLORBUTTON, 1)
#define lui_checkColorbutton(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_COLORBUTTON))

/*** Property
 * Object: colorbutton
 * Name: color
 * a table with r, g, b, a fields with values set to the currently selected
 * color. Defaults to 0 for each of r, g, b, a.
 *** Property
 * Object: colorbutton
 * Name: onchanged
 * a function <code>onchanged(colorbutton)</code> that is called when a new
 * color was selected.
 */
static int lui_colorbutton__index(lua_State *L)
{
	lui_object *lobj = lui_checkColorbutton(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "color") == 0) {
		double r, g, b, a;
		uiColorButtonColor(uiColorButton(lobj->object), &r, &g, &b, &a);
		lui_aux_pushRgbaAsTable(L, r, g, b, a);
	} else if (strcmp(what, "onchanged") == 0) {
		lui_objectGetHandler(L, "onchanged");
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_colorbutton__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkColorbutton(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "color") == 0) {
		double r, g, b, a;
		lui_aux_rgbaFromTable(L, 3, &r, &g, &b, &a);
		uiColorButtonSetColor(uiColorButton(lobj->object), r, g, b, a);
	} else if (strcmp(what, "onchanged") == 0) {
		lui_objectSetHandler(L, "onchanged", 3);
	} else {
		return lui_control__newindex(L);
	}
	return 0;
}

static void lui_colorbuttonOnChangedCallback(uiColorButton *btn, void *data)
{
	lua_State *L = (lua_State*) data;
	int top = lua_gettop(L);
	lui_objectHandlerCallback(L, uiControl(btn), "onchanged", top, 0, 0);
	lua_settop(L, top);
}

/*** Constructor
 * Object: colorbutton
 * Name: colorbutton
 * Signature: colorbutton = lui.colorbutton(properties = nil)
 * create a new colorbutton.
 */
static int lui_newColorButton(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);

	lui_object *lobj = lui_pushColorbutton(L);
	lobj->object = uiNewColorButton();
	uiColorButtonOnChanged(uiColorButton(lobj->object), lui_colorbuttonOnChangedCallback, L);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for colorbuttons */
static const luaL_Reg lui_colorbutton_meta[] = {
	{"__index", lui_colorbutton__index},
	{"__newindex", lui_colorbutton__newindex},
	{0, 0}
};

static const struct luaL_Reg lui_area_funcs [] ={
	{"area", lui_newArea},
	{"fontbutton", lui_newFontButton},
	{"colorbutton", lui_newColorButton},
	{0, 0}
};

static void lui_addAreaEnums(lua_State *L)
{
	int top = lua_gettop(L);
	if (lua_getfield(L, top, "enum") == LUA_TNIL) {
		lua_pop(L, 1);
		lua_newtable(L);
	}

	lui_makeAreaKeyModifierEnum(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, top + 1, "keymod");
	lua_setfield(L, LUA_REGISTRYINDEX, "lui_enumkeymod");

	lui_makeAreaKeyExtEnum(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, top + 1, "keyext");
	lua_setfield(L, LUA_REGISTRYINDEX, "lui_enumkeyext");

	luiMakeWindowEdgeEnum(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, top + 1, "edge");
	lua_setfield(L, LUA_REGISTRYINDEX, "lui_enumwinedge");

	lua_setfield(L, top, "enum");
}

static int lui_init_area(lua_State *L)
{
	luaL_setfuncs(L, lui_area_funcs, 0);

	lui_add_control_type(L, LUI_AREA, lui_area_methods, lui_area_meta);
	lui_add_control_type(L, LUI_FONTBUTTON, 0, lui_fontbutton_meta);
	lui_add_control_type(L, LUI_COLORBUTTON, 0, lui_colorbutton_meta);

	lui_addAreaEnums(L);

	return 1;
}
