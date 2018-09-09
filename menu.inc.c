/* lui.c
 *
 * lua binding to libui (https://github.com/andlabs/libui)
 *
 * Gunnar Zötl <gz@tset.de>, 2016
 * Released under MIT/X11 license. See file LICENSE for details.
 *
 * This file is included by lui.c
 */

/* menuitem control  *******************************************************/

/*** Object
 * Name: menuitem
 * a single menu item, as returned by the menu:append*() methods. This is
 * not a control, so it does not have the standard control methods and
 * properties.
 */
#define LUI_MENUITEM "lui_menuitem"
#define lui_pushMenuitem(L) lui_pushObject(L, LUI_MENUITEM, 1)
#define lui_checkMenuitem(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_MENUITEM))

/*** Property
 * Object: menuitem
 * Name: text
 * the text of the menu item, as passed into menu:append() or menu:appendcheckable().
 * The other menu:append*() methods fill this with default names. This is a read-only
 * property.
 *** Property
 * Object: menuitem
 * Name: enabled
 * true if the menuitem is / should be enabled, false if not. Default is true.
 *** Property
 * Object: menuitem
 * Name: checked
 * true if the checkable menu item is / should be checked, false if not.
 * Default is false.
 *** Property
 * Object: menuitem
 * Name: onclicked
 * a function <code>onclicked(menuitem, window)</code> that is called when
 * this menu item is clicked. Note that quit menuitems don't call their
 * onclicked handler, and checkable menu items call the handler after the
 * checked status has changed.
 */
static int lui_menuitem__index(lua_State *L)
{
	lui_object *lobj = lui_checkMenuitem(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "enabled") == 0) {
		if (lui_aux_getUservalue(L, 1, "enabled") == LUA_TNIL) {
			lua_pop(L, 1);
			lua_pushboolean(L, 1);
		}
	} else if (strcmp(what, "checked") == 0) {
		lua_pushboolean(L, uiMenuItemChecked(uiMenuItem(lobj->object)) != 0);
	} else if (strcmp(what, "text") == 0) {
		lui_aux_getUservalue(L, 1, "text");
	} else if (strcmp(what, "onclicked") == 0) {
		lui_objectGetHandler(L, "onclicked");
	} else {
		return lui_utility__index(L);
	}
	return 1;
}

static int lui_menuitem__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkMenuitem(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "enabled") == 0) {
		int enabled = lua_toboolean(L, 3);
		if (enabled) {
			uiMenuItemEnable(uiMenuItem(lobj->object));
		} else {
			uiMenuItemDisable(uiMenuItem(lobj->object));
		}
		lui_aux_setUservalue(L, 1, "enabled", enabled);
	} else if (strcmp(what, "checked") == 0) {
		int checked = lua_toboolean(L, 3);
		uiMenuItemSetChecked(uiMenuItem(lobj->object), checked);
	} else if (strcmp(what, "onclicked") == 0) {
		lui_objectSetHandler(L, "onclicked", 3);
	} else {
		return lui_multilineEntry__newindex(L);
	}
	return 0;
}

static int lui_menuitem__gc(lua_State *L)
{
	lui_object *lobj = lui_checkMenuitem(L, 1);
	lobj->object = 0;
	/* nothing else to do here */
	return 0;
}

static void lui_menuitemOnClickedCallback(uiMenuItem *mi, uiWindow *win, void *data)
{
	lua_State *L = (lua_State*) data;
	int top = lua_gettop(L);
	lui_findObject(L, uiControl(win));
	lui_objectHandlerCallback(L, uiControl(mi), "onclicked", top + 1, 1, 0);
	lua_settop(L, top);
}

static int lui_wrapMenuItem(lua_State *L, uiMenuItem *mi, const char *text, int nohandler)
{
	lui_object *lobj = lui_pushMenuitem(L);
	lobj->object = mi;
	lua_pushstring(L, text);
	lui_aux_setUservalue(L, lua_gettop(L) - 1, "text", lua_gettop(L));
	lua_pop(L, 1);
	if (nohandler == 0) {
		uiMenuItemOnClicked(mi, lui_menuitemOnClickedCallback, L);
		lui_registerObject(L, -1);
	}
	return 1;
}

/* metamethods for menuitem */
static const luaL_Reg lui_menuitem_meta[] = {
	{"__index", lui_menuitem__index},
	{"__newindex", lui_menuitem__newindex},
	{"__gc", lui_menuitem__gc},
	{0, 0}
};

/* menu control  ***********************************************************/

/*** Object
 * Name: menu
 * a menu for a window. All menus must be created before the window they
 * should be attached to. Once a window with menus has been created, you
 * can not create any more menus for other windows, and can safely drop the
 * reference to any created lui.menu's. This is not a control, so it does
 * not have the standard control methods and properties.
 */
#define LUI_MENU "lui_menu"
#define lui_pushMenu(L) lui_pushObject(L, LUI_MENU, 1)
#define lui_checkMenu(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_MENU))

static int lui_menu__gc(lua_State *L)
{
	lui_object *lobj = lui_checkMenu(L, 1);
	lobj->object = 0;
	return 0;
}

/*** Method
 * Object: menu
 * Name: append
 * Signature: menuitem = menu:append(text, menuitemproperties = nil)
 * append a standard item to a menu.
 */
static int lui_menuAppend(lua_State *L)
{
	lui_object *lobj = lui_checkMenu(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int hastable = lui_aux_istable(L, 3);

	uiMenuItem *mi = uiMenuAppendItem(uiMenu(lobj->object), name);
	int res = lui_wrapMenuItem(L, mi, name, 0);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 3); }
	return res;
}

/*** Method
 * Object: menu
 * Name: appendcheckable
 * Signature: menuitem = menu:appendcheckable(text, menuitemproperties = nil)
 * append a checkable item to a menu. Default status is unchecked.
 */
static int lui_menuAppendCheckable(lua_State *L)
{
	lui_object *lobj = lui_checkMenu(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int hastable = lui_aux_istable(L, 3);

	uiMenuItem *mi = uiMenuAppendCheckItem(uiMenu(lobj->object), name);
	int res = lui_wrapMenuItem(L, mi, name, 0);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 3); }
	return res;
}

/*** Method
 * Object: menu
 * Name: appendquit
 * Signature: menuitem = menu:appendquit(menuitemproperties = nil)
 * append a quit item to a menu. The text property for this item will be
 * "quit". quit menu items can not have an onclicked handler
 */
static int lui_menuAppendQuit(lua_State *L)
{
	lui_object *lobj = lui_checkMenu(L, 1);
	int hastable = lui_aux_istable(L, 2);
	uiMenuItem *mi = uiMenuAppendQuitItem(uiMenu(lobj->object));
	int res = lui_wrapMenuItem(L, mi, "quit", 1);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 2); }
	return res;
}

/*** Method
 * Object: menu
 * Name: appendpreferences
 * Signature: menuitem = menu:appendpreferences(menuitemproperties = nil)
 * append a preferences item to a menu. The text property for this item will
 * be "preferences".
 */
static int lui_menuAppendPreferences(lua_State *L)
{
	lui_object *lobj = lui_checkMenu(L, 1);
	int hastable = lui_aux_istable(L, 2);
	uiMenuItem *mi = uiMenuAppendPreferencesItem(uiMenu(lobj->object));
	int res = lui_wrapMenuItem(L, mi, "preferences", 0);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 2); }
	return res;
}

/*** Method
 * Object: menu
 * Name: appendabout
 * Signature: menuitem = menu:appendabout(menuitemproperties = nil)
 * append an about item to a menu. The text property for this item will be
 * "about".
 */
static int lui_menuAppendAbout(lua_State *L)
{
	lui_object *lobj = lui_checkMenu(L, 1);
	int hastable = lui_aux_istable(L, 2);
	uiMenuItem *mi = uiMenuAppendAboutItem(uiMenu(lobj->object));
	int res = lui_wrapMenuItem(L, mi, "about", 0);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 2); }
	return res;
}

/*** Method
 * Object: menu
 * Name: appendseparator
 * Signature: menuitem = menu:appendseparator()
 * append a separator to a menu.
 */
static int lui_menuAppendSeparator(lua_State *L)
{
	lui_object *lobj = lui_checkMenu(L, 1);
	uiMenuAppendSeparator(uiMenu(lobj->object));
	return 0;
}

/*** Constructor
 * Object: menu
 * Name: menu
 * Signature: menu = lui.menu(title)
 * create a new menu with title.
 */

static int lui_newMenu(lua_State *L)
{
	const char *title = luaL_checkstring(L, 1);
	lui_object *lobj = lui_pushMenu(L);
	lobj->object = uiNewMenu(title);
	return 1;
}

/* metamethods for menus */
static const luaL_Reg lui_menu_meta[] = {
	{"__gc", lui_menu__gc},
	{0, 0}
};

/* methods for menus */
static const luaL_Reg lui_menu_methods[] = {
	{"append", lui_menuAppend},
	{"appendcheckable", lui_menuAppendCheckable},
	{"appendquit", lui_menuAppendQuit},
	{"appendpreferences", lui_menuAppendPreferences},
	{"appendabout", lui_menuAppendAbout},
	{"appendseparator", lui_menuAppendSeparator},
	{0, 0}
};

/* menu function list table
 */
static const struct luaL_Reg lui_menu_funcs [] ={
	{"menu", lui_newMenu},
	{0, 0}
};

static int lui_init_menu(lua_State *L)
{
	luaL_setfuncs(L, lui_menu_funcs, 0);

	/* these are not controls! */
	lui_add_utility_type(L, LUI_MENUITEM, 0, lui_menuitem_meta);
	lui_add_utility_type(L, LUI_MENU, lui_menu_methods, lui_menu_meta);

	return 1;
}
