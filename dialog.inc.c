/* lui.c
 *
 * lua binding to libui (https://github.com/andlabs/libui)
 *
 * Gunnar Zötl <gz@tset.de>, 2016-2018
 * Released under MIT/X11 license. See file LICENSE for details.
 *
 * This file is included by lui.c
 */

typedef struct {
	int id;
	int *status;
} lui_dialogButtonId;

static int lui_dialogOnClosingCallback(uiWindow *win, void *data)
{
	int *status = (int*) data;
	*status = 0;
	return 0;
}

static void lui_dialogButtonOnClickedCallback(uiButton *btn, void *data)
{
	lui_dialogButtonId *id = (lui_dialogButtonId*) data;
	*(id->status) = id->id;
}

/*** Function
 * Name: dialog
 * Signature: number = lui.dialog([title], [w, h], control, [button1, ...])
 * open a dialog, and wait for it to complete. Returns the number of the
 * button that was clicked, starting from 1, or nil if the dialog was just
 * aborted. control should be a container, like a box or a form, that contains
 * the controls that make up the dialog.
 */
// TODO check whether we need to do this more lua-like... (i.e. allocate lua
// objects and use lua methods for stuff)
static int lui_runDialog(lua_State *L)
{
	int arg = 1;
	int narg = lua_gettop(L);
	int status = -1;

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

	lui_object *lobj = lui_checkObject(L, arg);

	uiBox *vbox = uiNewVerticalBox();
	uiBoxAppend(vbox, uiControl(lobj->object), 1);
	uiBox *hbox = uiNewHorizontalBox();
	uiBoxAppend(vbox, uiControl(hbox), 0);

	uiWindow *win = uiNewWindow(title, width, height, 0);
	uiWindowOnClosing(win, lui_dialogOnClosingCallback, &status);
	uiWindowSetChild(win, uiControl(vbox));

	/* lui_dialogButtonId buttonid[narg + 1]; */
	lui_dialogButtonId *buttonid = (lui_dialogButtonId*)malloc((narg + 1) * sizeof(lui_dialogButtonId));
	if (narg == arg) {
		uiButton *btn = uiNewButton("OK");
		uiButtonOnClicked(btn, lui_dialogButtonOnClickedCallback, &status);
		uiBoxAppend(hbox, uiControl(btn), 1);
	} else {
		++arg;
		for (int i = arg; i <= narg; ++i) {
			buttonid[i].id = i + 1 - arg;
			buttonid[i].status = &status;
			const char *bname = luaL_checkstring(L, i);
			uiButton *btn = uiNewButton(bname);
			uiButtonOnClicked(btn, lui_dialogButtonOnClickedCallback, &buttonid[i]);
			uiBoxAppend(hbox, uiControl(btn), 1);
		}
	}
	free(buttonid);

	uiControlShow(uiControl(win));
	while (uiMainStep(1) && status < 0) {
		/* nothing */
	}

	/* destroy everything except for the control we were passed as an argument */
	uiWindowSetChild(win, 0);
	uiControlDestroy(uiControl(win));
	uiBoxDelete(vbox, 0);
	uiControlDestroy(uiControl(vbox));

	if (status <= 0) {
		lua_pushnil(L);
	} else {
		lua_pushinteger(L, status);
	}

	return 1;
}

static const struct luaL_Reg lui_dialog_funcs [] ={
	{"dialog", lui_runDialog},
	{0, 0}
};

static int lui_init_dialog(lua_State *L)
{
	luaL_setfuncs(L, lui_dialog_funcs, 0);

	return 1;
}
