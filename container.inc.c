/* lui.c
 *
 * lua binding to libui (https://github.com/andlabs/libui)
 *
 * Gunnar Zötl <gz@tset.de>, 2016
 * Released under MIT/X11 license. See file LICENSE for details.
 *
 * This file is included by lui.c
 */

/* window control  *********************************************************/

/*** Object
 * Name: window
 */
#define LUI_WINDOW "lui_window"
#define lui_pushWindow(L) lui_pushObject(L, LUI_WINDOW, 1)
#define lui_checkWindow(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_WINDOW))

/*** Property
 * Object: window
 * Name: title
 * get / set the title of a window.
 * Object: window
 * Name: margined
 * true if the window has / should have a margin around its contents, false if
 * not. Default is false.
 *** Property
 * Object: window
 * Name: fullscreen
 * true if the window is / should be a fullscreen, false if not. Default is
 * false.
 *** Property
 * Name: borderless
 * true if the window is / should be borderless, false if not. Default is
 * false.
 *** Property
 * Object: window
 * Name: onclosing
 * a function <code>bool onclosing(window)</code> to be called when the user
 * requested to close the window. Must return true if the window can be
 * closed, false or nil if not.
 *** Property
 * Object: window
 * Name: oncontentsizechanged
 * a function <code>oncontentsizechanged(window)</code> to be called when the user
 * changes the window size.
 */
static int lui_window__index(lua_State *L)
{
	lui_object *lobj = lui_checkWindow(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "title") == 0) {
		char *title = uiWindowTitle(uiWindow(lobj->object));
		lua_pushstring(L, title);
		uiFreeText(title);
	} else if (strcmp(what, "margined") == 0) {
		lua_pushboolean(L, uiWindowMargined(uiWindow(lobj->object)) != 0);
	} else if (strcmp(what, "fullscreen") == 0) {
		lua_pushboolean(L, uiWindowFullscreen(uiWindow(lobj->object)) != 0);
	} else if (strcmp(what, "borderless") == 0) {
		lua_pushboolean(L, uiWindowBorderless(uiWindow(lobj->object)) != 0);
	} else if (strcmp(what, "oncontentsizechanged") == 0) {
		lui_objectGetHandler(L, "oncontentsizechanged");
	} else if (strcmp(what, "onclosing") == 0) {
		lui_objectGetHandler(L, "onclosing");
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_window__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkWindow(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "title") == 0) {
		const char *title = luaL_checkstring(L, 3);
		uiWindowSetTitle(uiWindow(lobj->object), title);
	} else if (strcmp(what, "margined") == 0) {
		int margined = lua_toboolean(L, 3);
		uiWindowSetMargined(uiWindow(lobj->object), margined);
	} else if (strcmp(what, "fullscreen") == 0) {
		int fullscreen = lua_toboolean(L, 3);
		uiWindowSetFullscreen(uiWindow(lobj->object), fullscreen);
	} else if (strcmp(what, "borderless") == 0) {
		int borderless = lua_toboolean(L, 3);
		uiWindowSetBorderless(uiWindow(lobj->object), borderless);
	} else if (strcmp(what, "oncontentsizechanged") == 0) {
		lui_objectSetHandler(L, "oncontentsizechanged", 3);
	} else if (strcmp(what, "onclosing") == 0) {
		lui_objectSetHandler(L, "onclosing", 3);
	} else {
		return lui_control__newindex(L);
	}
	return 1;
}

static void lui_windowOnContentSizeChangedCallback(uiWindow *win, void *data)
{
	lua_State *L = (lua_State*) data;
	int top = lua_gettop(L);
	lui_objectHandlerCallback(L, uiControl(win), "oncontentsizechanged", top, 0, 0);
	lua_settop(L, top);
}

static int lui_windowOnClosingCallback(uiWindow *win, void *data)
{
	lua_State *L = (lua_State*) data;
	int top = lua_gettop(L);
	int kill = 1;
	if (lui_objectHandlerCallback(L, uiControl(win), "onclosing", top, 0, 1) != 0) {
		kill = lua_toboolean(L, -1);
	}
	lua_settop(L, top);
	/* don't close, just hide! */
	if (kill && lui_findObject(L, uiControl(win)) != LUA_TNIL) {
		uiControlHide(uiControl(win));
	}
	lua_pop(L, 1);
	return 0;
}

/*** Method
 * Object: window
 * Name: contentsize
 * Signature: w, h = window:contentsize(h = nil, h = nil)
 * set and get the content size for a window. Sets the size if the arguments
 * w and h are not nil, returns the content size of the window.
 */
static int lui_windowContentsize(lua_State *L)
{
	lui_object *lobj = lui_checkWindow(L, 1);
	int w, h;
	if (!lua_isnoneornil(L, 2) && !lua_isnoneornil(L, 3)) {
		w = luaL_checkinteger(L, 2);
		h = luaL_checkinteger(L, 3);
		uiWindowSetContentSize(uiWindow(lobj->object), w, h);
	}
	uiWindowContentSize(uiWindow(lobj->object), &w, &h);
	lua_pushinteger(L, w);
	lua_pushinteger(L, h);
	return 2;
}

/*** Method
 * Object: window
 * Name: setchild
 * Signature: control = window:setchild(control)
 * set the child control for a window. Returns the child control.
 */
static int lui_windowSetChild(lua_State *L)
{
	lui_object *lobj = lui_checkWindow(L, 1);
	lui_object *child = lui_checkObject(L, 2);
	uiWindowSetChild(uiWindow(lobj->object), child->object);
	lui_controlSetChild(L, 1, 2);
	lua_pushvalue(L, 2);
	return 1;
}

/*** Method
 * Object: window
 * Name: hide
 * Signature: window:hide()
 * hides the window. This is the same as setting window.visible = false
 */
static int lui_windowHide(lua_State *L)
{
	lui_object *lobj = lui_checkWindow(L, 1);
	uiControlHide(lobj->object);
	return 0;
}

/*** Method
 * Object: window
 * Name: show
 * Signature: window:show()
 * shows the window. This is the same as setting window.visible = true
 */
static int lui_windowShow(lua_State *L)
{
	lui_object *lobj = lui_checkWindow(L, 1);
	uiControlShow(lobj->object);
	return 0;
}

/*** Constructor
 * Object: window
 * Name: window
 * Signature: win = lui.window(title = "(lui window"), [width = 600, height = 600], hasmenubar = false, properties = nil)
 * create a new window. Every parameter is optional, with the exception
 * that when you specify a width, you must also specify a height. If
 * hasmenubar is true, the menu must have been created before the window
 * is created.
 */
static int lui_newWindow(lua_State *L)
{
	int arg = 1;
	const char *title = LUI_DEFAULT_WINDOW_TITLE;
	if (lua_isstring(L, arg)) {
		title = lua_tostring(L, arg++);
	}
	int width = LUI_DEFAULT_WINDOW_WIDTH;
	int height = LUI_DEFAULT_WINDOW_HEIGHT;
	if (lua_isnumber(L, arg)) {
		width = luaL_checkinteger(L, arg++);
		height = luaL_checkinteger(L, arg++);
	}
	int hasmenubar = 0;
	if (lua_isboolean(L, arg)) {
		hasmenubar = lua_toboolean(L, arg++);
	}
	int hastable = lui_aux_istable(L, arg);

	lui_object *lobj = lui_pushWindow(L);
	lobj->object = uiNewWindow(title, width, height, hasmenubar);
	uiWindowOnContentSizeChanged(uiWindow(lobj->object), lui_windowOnContentSizeChangedCallback, L);
	uiWindowOnClosing(uiWindow(lobj->object), lui_windowOnClosingCallback, L);
	uiControlShow(uiControl(lobj->object));
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), arg); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for uiwindows */
static const luaL_Reg lui_window_meta[] = {
	{"__index", lui_window__index},
	{"__newindex", lui_window__newindex},
	{0, 0}
};

/* methods for uiWindows */
static const luaL_Reg lui_window_methods[] = {
	{"setchild", lui_windowSetChild},
	{"contentsize", lui_windowContentsize},
	{"hide", lui_windowHide},
	{"show", lui_windowShow},
	{0, 0}
};

/* box control  ************************************************************/

/*** Object
 * Name: box
 * a container for multiple controls. Automatically arranges contents
 * vertically (for a vbox) or horizontally (for a hbox).
 */
#define LUI_BOX "lui_box"
#define lui_pushBox(L) lui_pushObject(L, LUI_BOX, 1)
#define lui_checkBox(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_BOX))

/*** Property
 * Object: box
 * Name: padded
 * true if there is / should be padding between the boxes contained controls,
 * false if not. Default is false.
 */
static int lui_box__index(lua_State *L)
{
	lui_object *lobj = lui_checkBox(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "padded") == 0) {
		lua_pushinteger(L, uiBoxPadded(uiBox(lobj->object)));
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_box__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkBox(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "padded") == 0) {
		int padded = lua_toboolean(L, 3);
		uiBoxSetPadded(uiBox(lobj->object), padded);
	} else {
		return lui_control__newindex(L);
	}
	return 1;
}

/*** Method
 * Object: box
 * Name: append
 * Signature: control = box:append(control, stretchy = false)
 * append a control to a box. If stretchy is set to true, the control will
 * be expanded to fill all available space. Returns the appended control.
 */
static int lui_boxAppend(lua_State *L)
{
	lui_object *lobj = lui_checkBox(L, 1);
	lui_object *child = lui_checkObject(L, 2);
	int stretchy = lua_toboolean(L, 3);
	uiBoxAppend(uiBox(lobj->object), child->object, stretchy);
	lui_controlAppendChild(L, 1, 2);
	lua_pushvalue(L, 2);
	return 1;
}

/*** Method
 * Object: box
 * Name: delete
 * Signature: box:delete(index)
 * delete a control by index from a box. index starts at 1
 */
static int lui_boxDelete(lua_State *L)
{
	lui_object *lobj = lui_checkBox(L, 1);
	int pos = luaL_checkinteger(L, 2);
	uiBoxDelete(uiBox(lobj->object), pos - 1);
	lui_controlDelChild(L, 1, pos);
	return 0;
}

/*** Constructor
 * Object: box
 * Name: hbox
 * Signature: hbox = lui.hbox(properties = nil)
 * creates a new box whose children are arranged horizontally.
 */
static int lui_newHBox(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);

	lui_object *lobj = lui_pushBox(L);
	lobj->object = uiNewHorizontalBox();
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/*** Constructor
 * Object: box
 * Name: vbox
 * Signature: vbox = lui.vbox(properties = nil)
 * creates a new box whose children are arranged vertically.
 */
static int lui_newVBox(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);

	lui_object *lobj = lui_pushBox(L);
	lobj->object = uiNewVerticalBox();
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for boxes */
static const luaL_Reg lui_box_meta[] = {
	{"__index", lui_box__index},
	{"__newindex", lui_box__newindex},
	{0, 0}
};

/* methods for boxes */
static const luaL_Reg lui_box_methods[] = {
	{"append", lui_boxAppend},
	{"delete", lui_boxDelete},
	{0, 0}
};

/* tab controls  ***********************************************************/

// TODO tab margined objects, to allow for tab.margined[page]

/*** Object
 * Name: tab
 * a container whose children are arranged in individual tabs.
 */
#define LUI_TAB "lui_tab"
#define lui_pushTab(L) lui_pushObject(L, LUI_TAB, 1)
#define lui_checkTab(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_TAB))

/*** Property
 * Object: tab
 * Name: margined
 * (TODO) not implemented
 *** Property
 * Object: tab
 * Name: numpages
 * returns the number of pages in a tabbed container. This is a read-only
 * property.
 */
static int lui_tab__index(lua_State *L)
{
	lui_object *lobj = lui_checkTab(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "margined") == 0) {
		//lua_pushboolean(L, uiTabMargined(uiTab(lobj->object), page) != 0); TODO
		NOT_IMPLEMENTED;
	} else if (strcmp(what, "numpages") == 0) {
		lua_pushinteger(L, uiTabNumPages(uiTab(lobj->object)));
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_tab__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkTab(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "margined") == 0) {
		//int margined = lua_toboolean(L, 3);
		//uiLabelSetTabMargined(uiTab(lobj->object), page, margined); TODO
		(void)lobj;
		NOT_IMPLEMENTED;
	} else {
		return lui_control__newindex(L);
	}
	return 0;
}

/*** Method
 * Object: tab
 * Name: append
 * Signature: control = tab:append(title, control)
 * append a control to a tabbed container. This should be a container control
 * like a box. Returns the appended control.
 */
static int lui_tabAppend(lua_State *L)
{
	lui_object *lobj = lui_checkTab(L, 1);
	const char *title = luaL_checkstring(L, 2);
	lui_object *child = lui_checkObject(L, 3);
	uiTabAppend(uiTab(lobj->object), title, child->object);
	lui_controlAppendChild(L, 1, 3);
	lua_pushvalue(L, 3);
	return 1;
}

/*** Method
 * Object: tab
 * Name: insertat
 * Signature: control = tab:insertat(title, before, control)
 * insert a control before a position into a tabbed container. before is an
 * index starting at 1. Returns the inserted control.
 */
static int lui_tabInsertAt(lua_State *L)
{
	lui_object *lobj = lui_checkTab(L, 1);
	const char *title = luaL_checkstring(L, 2);
	int before = luaL_checkinteger(L, 3) - 1;
	lui_object *child = lui_checkObject(L, 4);
	uiTabInsertAt(uiTab(lobj->object), title, before, child->object);
	lui_controlInsertChild(L, 1, before, 4);
	lua_pushvalue(L, 4);
	return 1;
}

/*** Method
 * Object: tab
 * Name: delete
 * Signature: tab:delete(index)
 * delete a control and its tab by index from a tabbed container. index starts
 * at 1
 */
static int lui_tabDelete(lua_State *L)
{
	lui_object *lobj = lui_checkTab(L, 1);
	int pos = luaL_checkinteger(L, 2) - 1;
	uiTabDelete(uiTab(lobj->object), pos);
	lui_controlDelChild(L, 1, pos);
	return 0;
}

/*** Constructor
 * Object: tab
 * Name: tab
 * Signature: tab = lui.tab(properties = nil)
 * create a new tabbed container.
 */
static int lui_newTab(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);

	lui_object *lobj = lui_pushTab(L);
	lobj->object = uiNewTab();
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for tabs */
static const luaL_Reg lui_tab_meta[] = {
	{"__index", lui_tab__index},
	{"__newindex", lui_tab__newindex},
	{0, 0}
};

/* methods for tabs */
static const luaL_Reg lui_tab_methods[] = {
	{"append", lui_tabAppend},
	{"insertat", lui_tabInsertAt},
	{"delete", lui_tabDelete},
	{0, 0}
};

/* group control  **********************************************************/

/*** Object
 * Name: group
 * a labelled container control.
 */
#define LUI_GROUP "lui_group"
#define lui_pushGroup(L) lui_pushObject(L, LUI_GROUP, 1)
#define lui_checkGroup(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_GROUP))

/*** Property
 * Object: group
 * Name: title
 * the label of the group
 *** Property
 * Object: group
 * Name: margined
 * true if the group has / should have a margin around its contents, false if
 * not. Default is false.
 */
static int lui_group__index(lua_State *L)
{
	lui_object *lobj = lui_checkGroup(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "title") == 0) {
		char *text = uiGroupTitle(uiGroup(lobj->object));
		lua_pushstring(L, text);
		uiFreeText(text);
	} else if (strcmp(what, "margined") == 0) {
		lua_pushboolean(L, uiGroupMargined(uiGroup(lobj->object)) != 0);
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_group__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkGroup(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "title") == 0) {
		const char *title = lua_tostring(L, 3);
		uiGroupSetTitle(uiGroup(lobj->object), title);
	} else if (strcmp(what, "margined") == 0) {
		int margined = lua_toboolean(L, 3);
		uiGroupSetMargined(uiGroup(lobj->object), margined);
	} else {
		return lui_control__newindex(L);
	}
	return 0;
}

/*** Method
 * Object: group
 * Name: setchild
 * Signature: group:setchild(control)
 * set the child control for a group control. Returns the child control.
 */
static int lui_groupSetChild(lua_State *L)
{
	lui_object *lobj = lui_checkGroup(L, 1);
	lui_object *child = lui_checkObject(L, 2);
	uiGroupSetChild(uiGroup(lobj->object), child->object);
	lui_controlSetChild(L, 1, 2);
	lua_pushvalue(L, 2);
	return 1;
}

/*** Constructor
 * Object: group
 * Name: group
 * Signature: group = lui.group(title, properties = nil)
 * create a new group control.
 */
static int lui_newGroup(lua_State *L)
{
	const char *title = luaL_checkstring(L, 1);
	int hastable = lui_aux_istable(L, 2);

	lui_object *lobj = lui_pushGroup(L);
	lobj->object = uiNewGroup(title);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 2); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for groups */
static const luaL_Reg lui_group_meta[] = {
	{"__index", lui_group__index},
	{"__newindex", lui_group__newindex},
	{0, 0}
};

/* methods for groups */
static const luaL_Reg lui_group_methods[] = {
	{"setchild", lui_groupSetChild},
	{0, 0}
};

/* form control  ************************************************************/

/*** Object
 * Name: form
 * a container, which arranges its children vertically and aligns the labels
 * and controls.
 */
#define LUI_FORM "lui_form"
#define lui_pushForm(L) lui_pushObject(L, LUI_FORM, 1)
#define lui_checkForm(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_FORM))

/*** Property
 * Object: form
 * Name: padded
 * true if there is / should be padding between the forms children, false if
 * not. Default is false.
 */
static int lui_form__index(lua_State *L)
{
	lui_object *lobj = lui_checkForm(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "padded") == 0) {
		lua_pushinteger(L, uiFormPadded(uiForm(lobj->object)));
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_form__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkForm(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "padded") == 0) {
		int padded = lua_toboolean(L, 3);
		uiFormSetPadded(uiForm(lobj->object), padded);
	} else {
		return lui_control__newindex(L);
	}
	return 1;
}

/*** Method
 * Object: form
 * Name: append
 * Signature: control = form:append(label, control, stretchy = false)
 * append a control to a form. If stretchy is set to true, the control will
 * be expanded to fill all available space. Returns the appended control.
 */
static int lui_formAppend(lua_State *L)
{
	lui_object *lobj = lui_checkForm(L, 1);
	const char *label = luaL_checkstring(L, 2);
	lui_object *child = lui_checkObject(L, 3);
	int stretchy = lua_toboolean(L, 4);
	uiFormAppend(uiForm(lobj->object), label, child->object, stretchy);
	lui_controlAppendChild(L, 1, 3);
	lua_pushvalue(L, 3);
	return 1;
}

/*** Method
 * Object: form
 * Name: delete
 * Signature: form:delete(control, index)
 * delete a control by index from a form. index starts at 1.
 */
static int lui_formDelete(lua_State *L)
{
	lui_object *lobj = lui_checkForm(L, 1);
	int pos = luaL_checkinteger(L, 2);
	uiFormDelete(uiForm(lobj->object), pos - 1);
	lui_controlDelChild(L, 1, pos);
	return 0;
}

/*** Constructor
 * Object: form
 * Name: form
 * Signature: form = lui.form(properties = nil)
 * create a new form control.
 */
static int lui_newForm(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);

	lui_object *lobj = lui_pushForm(L);
	lobj->object = uiNewForm();
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for forms */
static const luaL_Reg lui_form_meta[] = {
	{"__index", lui_form__index},
	{"__newindex", lui_form__newindex},
	{0, 0}
};

/* methods for forms */
static const luaL_Reg lui_form_methods[] = {
	{"append", lui_formAppend},
	{"delete", lui_formDelete},
	{0, 0}
};

/* grid control  ************************************************************/

/*** Object
 * Name: grid
 * a container, which arranges its children in a grid.
 */
#define LUI_GRID "lui_grid"
#define lui_pushGrid(L) lui_pushObject(L, LUI_GRID, 1)
#define lui_checkGrid(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_GRID))

/*** Property
 * Object: grid
 * Name: padded
 * true if there is / should be padding between the grids children, false if
 * not. Default is false.
 */
static int lui_grid__index(lua_State *L)
{
	lui_object *lobj = lui_checkGrid(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "padded") == 0) {
		lua_pushinteger(L, uiGridPadded(uiGrid(lobj->object)));
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_grid__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkGrid(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "padded") == 0) {
		int padded = lua_toboolean(L, 3);
		uiGridSetPadded(uiGrid(lobj->object), padded);
	} else {
		return lui_control__newindex(L);
	}
	return 1;
}

static int lui_makeAlignEnum(lua_State *L)
{
	lua_newtable(L);
	lui_enumItem("fill", uiAlignFill);
	lui_enumItem("start", uiAlignStart);
	lui_enumItem("center", uiAlignCenter);
	lui_enumItem("end", uiAlignEnd);
	return 1;
}

static int lui_makeAtEnum(lua_State *L)
{
	lua_newtable(L);
	lui_enumItem("leading", uiAtLeading);
	lui_enumItem("top", uiAtTop);
	lui_enumItem("trailing", uiAtTrailing);
	lui_enumItem("bottom", uiAtBottom);
	return 1;
}

/*** Method
 * Object: grid
 * Name: append
 * Signature: control = grid:append(control, left, top, xspan = 1, yspan = 1, hexpand = false, halign = "fill", vexpand = false, valign = "fill")
 * append a control to a grid. left and top are the coordinates of the cell
 * where the control should be placed, starting at 1. xspan and yspan are
 * how many cells the control spans in either direction, minimum is 1. Set
 * hexpand to true to horizontally expand the control to fill available
 * space, false to not do that. Set halign to "fill", "start", "center" or
 * "end", or lui.enum.align.fill, ... to set horizontal alignment. vexpand
 * and valign are the same for the vertical case.
 */
static int lui_gridAppend(lua_State *L)
{
	lui_object *lobj = lui_checkGrid(L, 1);
	lui_object *child = lui_checkObject(L, 2);
	int left = luaL_checkinteger(L, 3) - 1;
	int top = luaL_checkinteger(L, 4) - 1;
	int xspan = luaL_optinteger(L, 5, 1);
	int yspan = luaL_optinteger(L, 6, 1);
	int hexpand = lua_toboolean(L, 7);
	int halign = uiAlignFill;
	if (lua_gettop(L) >= 8) {
		halign = lui_aux_getNumberOrValue(L, 8, "lui_enumalign");
	}
	int vexpand = lua_toboolean(L, 9);
	int valign = uiAlignFill;
	if (lua_gettop(L) >= 10) {
		valign = lui_aux_getNumberOrValue(L, 10, "lui_enumalign");
	}
	uiGridAppend(uiGrid(lobj->object), uiControl(child->object), left, top, xspan, yspan, hexpand, halign, vexpand, valign);
	lui_controlAppendChild(L, 1, 2);
	lua_pushvalue(L, 2);
	return 1;
}

/*** Method
 * Object: grid
 * Name: insertat
 * Signature: control = grid:insertat(control, existing, at, xspan = 1, yspan = 1, hexpand = false, halign = "fill", vexpand = false, valign = "fill")
 * insert a control into a grid relative to another control that already
 * exists in the grid. at is any of "leading", "trailing", "top" or "bottom",
 * or lui.enum.at.leading, .... The other arguments have the same meaning as
 * with grid:append().
 */
static int lui_gridInsertAt(lua_State *L)
{
	lui_object *lobj = lui_checkGrid(L, 1);
	lui_object *child = lui_checkObject(L, 2);
	lui_object *existing = lui_checkObject(L, 3);
	int at = lui_aux_getNumberOrValue(L, 4, "lui_enumat");
	int xspan = luaL_optinteger(L, 5, 1);
	int yspan = luaL_optinteger(L, 6, 1);
	int hexpand = lua_toboolean(L, 7);
	int halign = uiAlignFill;
	if (lua_gettop(L) >= 8) {
		halign = lui_aux_getNumberOrValue(L, 8, "lui_enumalign");
	}
	int vexpand = lua_toboolean(L, 9);
	int valign = uiAlignFill;
	if (lua_gettop(L) >= 10) {
		valign = lui_aux_getNumberOrValue(L, 10, "lui_enumalign");
	}
	uiGridInsertAt(uiGrid(lobj->object), uiControl(child->object), uiControl(existing->object), at, xspan, yspan, hexpand, halign, vexpand, valign);
	lui_controlAppendChild(L, 1, 2);
	lua_pushvalue(L, 2);
	return 1;
}

/*** Constructor
 * Object: grid
 * Name: grid
 * Signature: grid = lui.grid(properties = nil)
 * create a new grid control.
 */
static int lui_newGrid(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);

	lui_object *lobj = lui_pushGrid(L);
	lobj->object = uiNewGrid();
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for grids */
static const luaL_Reg lui_grid_meta[] = {
	{"__index", lui_grid__index},
	{"__newindex", lui_grid__newindex},
	{0, 0}
};

/* methods for grids */
static const luaL_Reg lui_grid_methods[] = {
	{"append", lui_gridAppend},
	{"insertat", lui_gridInsertAt},
	{0, 0}
};

/* container function list table
 */
static const struct luaL_Reg lui_container_funcs [] ={
	{"window", lui_newWindow},
	{"hbox", lui_newHBox},
	{"vbox", lui_newVBox},
	{"tab", lui_newTab},
	{"group", lui_newGroup},
	{"form", lui_newForm},
	{"grid", lui_newGrid},
	{0, 0}
};

static void lui_addContainerEnums(lua_State *L)
{
	int top = lua_gettop(L);
	if (lua_getfield(L, top, "enum") == LUA_TNIL) {
		lua_pop(L, 1);
		lua_newtable(L);
	}

	lui_makeAlignEnum(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, top + 1, "align");
	lua_setfield(L, LUA_REGISTRYINDEX, "lui_enumalign");

	lui_makeAtEnum(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, top + 1, "at");
	lua_setfield(L, LUA_REGISTRYINDEX, "lui_enumat");

	lua_setfield(L, top, "enum");
}

static int lui_init_container(lua_State *L)
{
	luaL_setfuncs(L, lui_container_funcs, 0);

	lui_add_control_type(L, LUI_WINDOW, lui_window_methods, lui_window_meta);
	lui_add_control_type(L, LUI_BOX, lui_box_methods, lui_box_meta);
	lui_add_control_type(L, LUI_TAB, lui_tab_methods, lui_tab_meta);
	lui_add_control_type(L, LUI_GROUP, lui_group_methods, lui_group_meta);
	lui_add_control_type(L, LUI_FORM, lui_form_methods, lui_form_meta);
	lui_add_control_type(L, LUI_GRID, lui_grid_methods, lui_grid_meta);

	lui_addContainerEnums(L);

	return 1;
}
