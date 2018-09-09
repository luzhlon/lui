/* lui.c
 *
 * lua binding to libui (https://github.com/andlabs/libui)
 *
 * Gunnar Zötl <gz@tset.de>, 2016
 * Released under MIT/X11 license. See file LICENSE for details.
 *
 * This file is included by lui.c
 */

/* button control  *********************************************************/

/*** Object
 * Name: button
 * a button control
 */
#define LUI_BUTTON "lui_button"
#define lui_pushButton(L) lui_pushObject(L, LUI_BUTTON, 1)
#define lui_checkButton(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_BUTTON))

/*** Property
 * Object: button
 * Name: text
 * the text on the button
 *** Property
 * Object: button
 * Name: onclicked
 * a function <code>onclicked(button)</code> that is called when the button
 * is clicked by the user.
 */
static int lui_button__index(lua_State *L)
{
	lui_object *lobj = lui_checkButton(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "text") == 0) {
		char *text = uiButtonText(uiButton(lobj->object));
		lua_pushstring(L, text);
		uiFreeText(text);
	} else if (strcmp(what, "onclicked") == 0) {
		lui_objectGetHandler(L, "onclicked");
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_button__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkButton(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "text") == 0) {
		const char *text = luaL_checkstring(L, 3);
		uiButtonSetText(uiButton(lobj->object), text);
	} else if (strcmp(what, "onclicked") == 0) {
		lui_objectSetHandler(L, "onclicked", 3);
	} else {
		return lui_control__newindex(L);
	}
	return 1;
}

static void lui_buttonOnClickedCallback(uiButton *btn, void *data)
{
	lua_State *L = (lua_State*) data;
	int top = lua_gettop(L);
	lui_objectHandlerCallback(L, uiControl(btn), "onclicked", top, 0, 1);
	lua_settop(L, top);
}

/*** Constructor
 * Object: button
 * Name: button
 * Signature: btn = lui.button(text, properties = nil)
 * create a new button. text is the text on the button.
 */
static int lui_newButton(lua_State *L)
{
	const char *text = luaL_checkstring(L, 1);
	int hastable = lui_aux_istable(L, 2);

	lui_object *lobj = lui_pushButton(L);
	lobj->object =  uiNewButton(text);
	uiButtonOnClicked(uiButton(lobj->object), lui_buttonOnClickedCallback, L);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 2); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for uibuttons */
static const luaL_Reg lui_button_meta[] = {
	{"__index", lui_button__index},
	{"__newindex", lui_button__newindex},
	{0, 0}
};

/* entry control  **********************************************************/

/*** Object
 * Name: entry
 * an entry control.
 */
#define LUI_ENTRY "lui_entry"
#define lui_pushEntry(L) lui_pushObject(L, LUI_ENTRY, 1)
#define lui_checkEntry(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_ENTRY))

/*** Property
 * Object: entry
 * Name: text
 * the text the entry contains.
 *** Property
 * Object: entry
 * Name: readonly
 * true if the entry control is / should be read only, false if not.
 *** Property
 * Object: entry
 * Name: onchanged
 * a function <code>onchanged(entry)</code> that is called when the contents
 * of the entry is changed by the user.
 */
static int lui_entry__index(lua_State *L)
{
	lui_object *lobj = lui_checkEntry(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "text") == 0) {
		char *text = uiEntryText(uiEntry(lobj->object));
		lua_pushstring(L, text);
		uiFreeText(text);
	} else if (strcmp(what, "readonly") == 0) {
		lua_pushboolean(L, uiEntryReadOnly(uiEntry(lobj->object)) != 0);
	} else if (strcmp(what, "onchanged") == 0) {
		lui_objectGetHandler(L, "onchanged");
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_entry__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkEntry(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "text") == 0) {
		const char *text = lua_tostring(L, 3);
		uiEntrySetText(uiEntry(lobj->object), text);
	} else if (strcmp(what, "readonly") == 0) {
		int readonly = lua_toboolean(L, 3);
		uiEntrySetReadOnly(uiEntry(lobj->object), readonly);
	} else if (strcmp(what, "onchanged") == 0) {
		lui_objectSetHandler(L, "onchanged", 3);
	} else {
		return lui_control__newindex(L);
	}
	return 0;
}

static void lui_entryOnChangedCallback(uiEntry *btn, void *data)
{
	lua_State *L = (lua_State*) data;
	int top = lua_gettop(L);
	lui_objectHandlerCallback(L, uiControl(btn), "onchanged", top, 0, 1);
	lua_settop(L, top);
}

static int lui_newBasicEntry(lua_State *L, uiEntry* (construct)(void))
{
	int hastable = lui_aux_istable(L, 1);

	lui_object *lobj = lui_pushEntry(L);
	lobj->object = construct();
	uiEntryOnChanged(uiEntry(lobj->object), lui_entryOnChangedCallback, L);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/*** Constructor
 * Object: entry
 * Name: entry
 * Signature: entry = lui.entry(properties = nil)
 * create a new entry control.
 */
static int lui_newEntry(lua_State *L)
{
	return lui_newBasicEntry(L, uiNewEntry);
}

/*** Constructor
 * Object: entry
 * Name: passwordentry
 * Signature: entry = lui.passwordentry(properties = nil)
 * create a new entry control that is suitable for entering passwords.
 */
static int lui_newPasswordEntry(lua_State *L)
{
	return lui_newBasicEntry(L, uiNewPasswordEntry);
}

/*** Constructor
 * Object: entry
 * Name: searchentry
 * Signature: entry = lui.searchentry(properties = nil)
 * create a new entry control that is suitable to be used as a search entry.
 */
static int lui_newSearchEntry(lua_State *L)
{
	return lui_newBasicEntry(L, uiNewSearchEntry);
}

/* metamethods for entries */
static const luaL_Reg lui_entry_meta[] = {
	{"__index", lui_entry__index},
	{"__newindex", lui_entry__newindex},
	{0, 0}
};

/* checkbox control  *******************************************************/

/*** Object
 * Name: checkbox
 * a checkbox control.
 */
#define LUI_CHECKBOX "lui_checkbox"
#define lui_pushCheckbox(L) lui_pushObject(L, LUI_CHECKBOX, 1)
#define lui_checkCheckbox(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_CHECKBOX))

/*** Property
 * Object: checkbox
 * Name: text
 * the label of the checkbox.
 *** Property
 * Object: checkbox
 * Name: checked
 * true if the entry is / should be chacked, false if not.
 *** Property
 * Object: checkbox
 * Name: ontoggled
 * a function <code>ontoggled(checkbox)</code> that is called when the
 * checkbox is toggled by the user.
 */
static int lui_checkbox__index(lua_State *L)
{
	lui_object *lobj = lui_checkCheckbox(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "text") == 0) {
		char *text = uiCheckboxText(uiCheckbox(lobj->object));
		lua_pushstring(L, text);
		uiFreeText(text);
	} else if (strcmp(what, "checked") == 0) {
		lua_pushboolean(L, uiCheckboxChecked(uiCheckbox(lobj->object)) != 0);
	} else if (strcmp(what, "ontoggled") == 0) {
		lui_objectGetHandler(L, "ontoggled");
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_checkbox__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkCheckbox(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "text") == 0) {
		const char *text = lua_tostring(L, 3);
		uiCheckboxSetText(uiCheckbox(lobj->object), text);
	} else if (strcmp(what, "checked") == 0) {
		int checked = lua_toboolean(L, 3);
		uiCheckboxSetChecked(uiCheckbox(lobj->object), checked);
	} else if (strcmp(what, "ontoggled") == 0) {
		lui_objectSetHandler(L, "ontoggled", 3);
	} else {
		return lui_control__newindex(L);
	}
	return 0;
}

static void lui_entryOnToggledCallback(uiCheckbox *btn, void *data)
{
	lua_State *L = (lua_State*) data;
	int top = lua_gettop(L);
	lui_objectHandlerCallback(L, uiControl(btn), "ontoggled", top, 0, 1);
	lua_settop(L, top);
}

/*** Constructor
 * Object: checkbox
 * Name: checkbox
 * Signature: checkbox = lui.checkbox(text, properties = nil)
 * create a new checkbox with label text.
 */
static int lui_newCheckbox(lua_State *L)
{
	const char *text = luaL_checkstring(L, 1);
	int hastable = lui_aux_istable(L, 2);

	lui_object *lobj = lui_pushCheckbox(L);
	lobj->object = uiNewCheckbox(text);
	uiCheckboxOnToggled(uiCheckbox(lobj->object), lui_entryOnToggledCallback, L);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 2); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for checkboxes */
static const luaL_Reg lui_checkbox_meta[] = {
	{"__index", lui_checkbox__index},
	{"__newindex", lui_checkbox__newindex},
	{0, 0}
};

/* label control  **********************************************************/

/*** Object
 * Name: label
 * a label control.
 */
#define LUI_LABEL "lui_label"
#define lui_pushLabel(L) lui_pushObject(L, LUI_LABEL, 1)
#define lui_checkLabel(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_LABEL))

/*** Property
 * Object: label
 * Name: text
 * the text on the label.
 */
static int lui_label__index(lua_State *L)
{
	lui_object *lobj = lui_checkLabel(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "text") == 0) {
		char *text = uiLabelText(uiLabel(lobj->object));
		lua_pushstring(L, text);
		uiFreeText(text);
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_label__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkLabel(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "text") == 0) {
		const char *text = lua_tostring(L, 3);
		uiLabelSetText(uiLabel(lobj->object), text);
	} else {
		return lui_control__newindex(L);
	}
	return 0;
}

/*** Constructor
 * Object: label
 * Name: label
 * Signature: label = lui.label(text)
 * create a new label.
 */
static int lui_newLabel(lua_State *L)
{
	const char *text = luaL_checkstring(L, 1);

	lui_object *lobj = lui_pushLabel(L);
	lobj->object = uiNewLabel(text);
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for labels */
static const luaL_Reg lui_label_meta[] = {
	{"__index", lui_label__index},
	{"__newindex", lui_label__newindex},
	{0, 0}
};

/* spinbox control  ********************************************************/

/*** Object
 * Name: spinbox
 * a spinbox control.
 */
#define LUI_SPINBOX "lui_spinbox"
#define lui_pushSpinbox(L) lui_pushObject(L, LUI_SPINBOX, 1)
#define lui_checkSpinbox(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_SPINBOX))

/*** Property
 * Object: spinbox
 * Name: value
 * the current value of the spinbox control.
 *** Property
 * Object: spinbox
 * Name: onchanged
 * a function <code>function(spinbox)</code> that is called when the value
 * of the spinbox is changed by the user.
 */
static int lui_spinbox__index(lua_State *L)
{
	lui_object *lobj = lui_checkSpinbox(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "value") == 0) {
		lua_pushinteger(L, uiSpinboxValue(uiSpinbox(lobj->object)));
	} else if (strcmp(what, "onchanged") == 0) {
		lui_objectGetHandler(L, "onchanged");
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_spinbox__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkSpinbox(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "value") == 0) {
		int value = luaL_checkinteger(L, 3);
		uiSpinboxSetValue(uiSpinbox(lobj->object), value);
	} else if (strcmp(what, "onchanged") == 0) {
		lui_objectSetHandler(L, "onchanged", 3);
	} else {
		return lui_control__newindex(L);
	}
	return 0;
}

static void lui_spinboxOnChangedCallback(uiSpinbox *spb, void *data)
{
	lua_State *L = (lua_State*) data;
	int top = lua_gettop(L);
	lui_objectHandlerCallback(L, uiControl(spb), "onchanged", top, 0, 1);
	lua_settop(L, top);
}

/*** Constructor
 * Object: spinbox
 * Name: spinbox
 * Signature: spinbox = lui.spinbox(min, max, properties = nil)
 * create a new spinbox where the value can be changed between integers min
 * and max (both inclusive).
 */
static int lui_newSpinbox(lua_State *L)
{
	int min = luaL_checkinteger(L, 1);
	int max = luaL_checkinteger(L, 2);
	int hastable = lui_aux_istable(L, 3);

	lui_object *lobj = lui_pushSpinbox(L);
	lobj->object = uiNewSpinbox(min, max);
	uiSpinboxOnChanged(uiSpinbox(lobj->object), lui_spinboxOnChangedCallback, L);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 3); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for groups */
static const luaL_Reg lui_spinbox_meta[] = {
	{"__index", lui_spinbox__index},
	{"__newindex", lui_spinbox__newindex},
	{0, 0}
};

/* progressbar control  ****************************************************/

/*** Object
 * Name: progressbar
 * a progressbar control.
 */
#define LUI_PROGRESSBAR "lui_progressbar"
#define lui_pushProgressbar(L) lui_pushObject(L, LUI_PROGRESSBAR, 1)
#define lui_checkProgressbar(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_PROGRESSBAR))

/*** Property
 * Object: progressbar
 * Name: value
 * the value (0-100) of the progress bar. Set to -1 for an indeterminate
 * value.
 */
static int lui_progressbar__index(lua_State *L)
{
	lui_object *lobj = lui_checkProgressbar(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "value") == 0) {
		lua_pushinteger(L, uiProgressBarValue(uiProgressBar(lobj->object)));
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_progressbar__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkProgressbar(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "value") == 0) {
		int value = luaL_checkinteger(L, 3);
		if (value < 0 || value > 100) {
			value = -1;
		}
		uiProgressBarSetValue(uiProgressBar(lobj->object), value);
	} else {
		return lui_control__newindex(L);
	}
	return 0;
}

/*** Constructor
 * Object: progressbar
 * Name: progressbar
 * Signature: progressbar = lui.progressbar(properties = nil)
 * create a new progressbar control.
 */
static int lui_newProgressbar(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);

	lui_object *lobj = lui_pushProgressbar(L);
	lobj->object = uiNewProgressBar();
	lua_pushinteger(L, 0);
	lui_aux_setUservalue(L, lua_gettop(L) - 1, "value", lua_gettop(L));
	lua_pop(L, 1);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for groups */
static const luaL_Reg lui_progressbar_meta[] = {
	{"__index", lui_progressbar__index},
	{"__newindex", lui_progressbar__newindex},
	{0, 0}
};

/* slider control  *********************************************************/

/*** Object
 * Name: slider
 * a slider control.
 */
#define LUI_SLIDER "lui_slider"
#define lui_pushSlider(L) lui_pushObject(L, LUI_SLIDER, 1)
#define lui_checkSlider(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_SLIDER))

/*** Property
 * Object: slider
 * Name: value
 * the value of the slider.
 *** Property
 * Object: slider
 * Name: onchanged
 * a function <code>onchanged(slider)</code> that is called when the value of
 * the slider is changed by the user.
 */
static int lui_slider__index(lua_State *L)
{
	lui_object *lobj = lui_checkSlider(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "value") == 0) {
		lua_pushinteger(L, uiSliderValue(uiSlider(lobj->object)));
	} else if (strcmp(what, "onchanged") == 0) {
		lui_objectGetHandler(L, "onchanged");
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_slider__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkSlider(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "value") == 0) {
		int value = luaL_checkinteger(L, 3);
		uiSliderSetValue(uiSlider(lobj->object), value);
	} else if (strcmp(what, "onchanged") == 0) {
		lui_objectSetHandler(L, "onchanged", 3);
	} else {
		return lui_control__newindex(L);
	}
	return 0;
}

static void lui_sliderOnChangedCallback(uiSlider *spb, void *data)
{
	lua_State *L = (lua_State*) data;
	int top = lua_gettop(L);
	lui_objectHandlerCallback(L, uiControl(spb), "onchanged", top, 0, 1);
	lua_settop(L, top);
}

/*** Constructor
 * Object: slider
 * Name: slider
 * Signature: slider = lui.slider(min, max, properties = nil)
 * create a new slider where the value can be changed between integers min
 * and max.
 */
static int lui_newSlider(lua_State *L)
{
	int min = luaL_checkinteger(L, 1);
	int max = luaL_checkinteger(L, 2);
	int hastable = lui_aux_istable(L, 3);

	lui_object *lobj = lui_pushSlider(L);
	lobj->object = uiNewSlider(min, max);
	uiSliderOnChanged(uiSlider(lobj->object), lui_sliderOnChangedCallback, L);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 3); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for groups */
static const luaL_Reg lui_slider_meta[] = {
	{"__index", lui_slider__index},
	{"__newindex", lui_slider__newindex},
	{0, 0}
};

/* separator control  ******************************************************/

/*** Object
 * Name: separator
 * a separator control
 */
#define LUI_SEPARATOR "lui_separator"
#define lui_pushSeparator(L) lui_pushObject(L, LUI_SEPARATOR, 1)

static int lui_newSeparator(lua_State *L, int isvertical)
{
	lui_object *lobj = lui_pushSeparator(L);
	lobj->object = isvertical ? uiNewVerticalSeparator() : uiNewHorizontalSeparator();
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/*** Constructor
 * Object: separator
 * Name: hseparator
 * Signature: separator = lui.hseparator(vertical = false)
 * create a new horizontal separator control.
 */
static int lui_newHSeparator(lua_State *L)
{
	return lui_newSeparator(L, 0);
}

/*** Constructor
 * Object: separator
 * Name: vseparator
 * Signature: separator = lui.vseparator(vertical = false)
 * create a new vertical separator control.
 */
static int lui_newVSeparator(lua_State *L)
{
	return lui_newSeparator(L, 0);
}

/* combobox control  *******************************************************/

/*** Object
 * Name: combobox
 * a combobox control.
 */
#define LUI_COMBOBOX "lui_combobox"
#define lui_pushCombobox(L) lui_pushObject(L, LUI_COMBOBOX, 1)
#define lui_checkCombobox(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_COMBOBOX))

/*** Property
 * Object: combobox
 * Name: selected
 * the index of the selected option.
 *** Property
 * Object: combobox
 * Name: text
 * the text of the selected option. This is a read-only property.
 *** Property
 * Object: combobox
 * Name: onselected
 * a function <code>onselected(combobox)</code> that is called when the value
 * of the combobox is changed by the user.
 */
static int lui_combobox__index(lua_State *L)
{
	lui_object *lobj = lui_checkCombobox(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "selected") == 0) {
		int selected = uiComboboxSelected(uiCombobox(lobj->object)) + 1;
		if (selected == 0) {
			lua_pushnil(L);
		} else {
			lua_pushinteger(L, selected);
		}
	} else if (strcmp(what, "text") == 0) {
		int which = uiComboboxSelected(uiCombobox(lobj->object)) + 1;
		if (which > 0) {
			if (lui_aux_getUservalue(L, 1, "values") != LUA_TNIL) {
				lua_rawgeti(L, -1, which);
			}
		}
	} else if (strcmp(what, "onselected") == 0) {
		lui_objectGetHandler(L, "onselected");
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_combobox__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkCombobox(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "selected") == 0) {
		int value = luaL_checkinteger(L, 3) - 1;
		uiComboboxSetSelected(uiCombobox(lobj->object), value);
	} else if (strcmp(what, "onselected") == 0) {
		lui_objectSetHandler(L, "onselected", 3);
	} else {
		return lui_control__newindex(L);
	}
	return 0;
}

/*** Method
 * Object: combobox
 * Name: append
 * Signature: combobox:append(entry, ...)
 * append one or more entries to a combobox.
 */
static int lui_comboboxAppend(lua_State *L)
{
	lui_object *lobj = lui_checkCombobox(L, 1);
	const char *text = lua_tostring(L, 2);
	if (lui_aux_getUservalue(L, 1, "values") == LUA_TNIL) {
		lua_pop(L, 1);
		lua_newtable(L);
	}
	int tbl = lua_gettop(L);
	uiComboboxAppend(uiCombobox(lobj->object), text);
	lui_aux_tinsert(L, tbl, -1, 2);
	for (int i = 3; i < tbl; ++i) {
		text = lua_tostring(L, i);
		uiComboboxAppend(uiCombobox(lobj->object), text);
		lui_aux_tinsert(L, tbl, -1, i);
	}
	lui_aux_setUservalue(L, 1, "values", tbl);
	lua_pop(L, 1);
	return 0;
}

static void lui_comboboxOnSelectedCallback(uiCombobox *spb, void *data)
{
	lua_State *L = (lua_State*) data;
	int top = lua_gettop(L);
	lui_objectHandlerCallback(L, uiControl(spb), "onselected", top, 0, 1);
	lua_settop(L, top);
}

/*** Constructor
 * Object: combobox
 * Name: combobox
 * Signature: combobox = lui.combobox(properties = nil)
 * create a new combobox control.
 */
static int lui_newCombobox(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);

	lui_object *lobj = lui_pushCombobox(L);
	lobj->object = uiNewCombobox();
	uiComboboxOnSelected(uiCombobox(lobj->object), lui_comboboxOnSelectedCallback, L);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for comboboxes */
static const luaL_Reg lui_combobox_meta[] = {
	{"__index", lui_combobox__index},
	{"__newindex", lui_combobox__newindex},
	{0, 0}
};

/* methods for comboboxes */
static const luaL_Reg lui_combobox_methods[] = {
	{"append", lui_comboboxAppend},
	{0, 0}
};

/* editable combobox control  **********************************************/

/*** Object
 * Name: editablecombobox
 * an editable combobox control.
 */
#define LUI_EDITABLECOMBOBOX "lui_editablecombobox"
#define lui_pushEditableCombobox(L) lui_pushObject(L, LUI_EDITABLECOMBOBOX, 1)
#define lui_checkEditableCombobox(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_EDITABLECOMBOBOX))

/*** Property
 * Object: editablecombobox
 * Name: text
 * the current value of the editable combobox.
 *** Property
 * Object: editablecombobox
 * Name: onchanged
 * a function <code>onchanged(editablecombobox)</code> that is called when
 * the value of the editable combobox is changed by the user.
 */
static int lui_editableCombobox__index(lua_State *L)
{
	lui_object *lobj = lui_checkEditableCombobox(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "text") == 0) {
		char *text = uiEditableComboboxText(uiEditableCombobox(lobj->object));
		lua_pushstring(L, text);
		uiFreeText(text);
	} else if (strcmp(what, "onchanged") == 0) {
		lui_objectGetHandler(L, "onchanged");
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_editableCombobox__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkEditableCombobox(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "text") == 0) {
		const char *text = luaL_checkstring(L, 3);
		uiEditableComboboxSetText(uiEditableCombobox(lobj->object), text);
	} else if (strcmp(what, "onchanged") == 0) {
		lui_objectSetHandler(L, "onchanged", 3);
	} else {
		return lui_control__newindex(L);
	}
	return 0;
}

/*** Method
 * Object: editablecombobox
 * Name: append
 * Signature: editablecombobox:append(entry, ...)
 * append one or more entries to an editable combobox.
 */
static int lui_editableComboboxAppend(lua_State *L)
{
	lui_object *lobj = lui_checkEditableCombobox(L, 1);
	const char *text = lua_tostring(L, 2);
	uiEditableComboboxAppend(uiEditableCombobox(lobj->object), text);
	for (int i = 3; i <= lua_gettop(L); ++i) {
		text = lua_tostring(L, i);
		uiEditableComboboxAppend(uiEditableCombobox(lobj->object), text);
	}
	return 0;
}

static void lui_editableComboboxOnChangedCallback(uiEditableCombobox *spb, void *data)
{
	lua_State *L = (lua_State*) data;
	int top = lua_gettop(L);
	lui_objectHandlerCallback(L, uiControl(spb), "onchanged", top, 0, 1);
	lua_settop(L, top);
}

/*** Constructor
 * Object: editablecombobox
 * Name: editablecombobox
 * Signature: editablecombobox = lui.editablecombobox(properties = nil)
 * create a new editable combobox.
 */
static int lui_newEditableCombobox(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);

	lui_object *lobj = lui_pushEditableCombobox(L);
	lobj->object = uiNewEditableCombobox();
	uiEditableComboboxOnChanged(uiEditableCombobox(lobj->object), lui_editableComboboxOnChangedCallback, L);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for editable comboboxes */
static const luaL_Reg lui_editableCombobox_meta[] = {
	{"__index", lui_editableCombobox__index},
	{"__newindex", lui_editableCombobox__newindex},
	{0, 0}
};

/* methods for editable comboboxes */
static const luaL_Reg lui_editableCombobox_methods[] = {
	{"append", lui_editableComboboxAppend},
	{0, 0}
};

/* radio controls  *********************************************************/

/*** Object
 * Name: radiobuttons
 * a radiobuttons control.
 */
#define LUI_RADIOBUTTONS "lui_radiobuttons"
#define lui_pushRadiobuttons(L) lui_pushObject(L, LUI_RADIOBUTTONS, 1)
#define lui_checkRadiobuttons(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_RADIOBUTTONS))

/*** Property
 * Object: radiobuttons
 * Name: selected
 * the index of the selected option.
 *** Property
 * Object: radiobuttons
 * Name: text
 * the text of the selected option. This is a read-only property.
 *** Property
 * Object: radiobuttons
 * Name: onselected
 * a function <code>onselected(radiobuttons)</code> that is called when the
 * value of the radiobuttons is changed by the user.
 */
static int lui_radiobuttons__index(lua_State *L)
{
	lui_object *lobj = lui_checkRadiobuttons(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "selected") == 0) {
		int selected = uiRadioButtonsSelected(uiRadioButtons(lobj->object)) + 1;
		if (selected == 0) {
			lua_pushnil(L);
		} else {
			lua_pushinteger(L, selected);
		}
	} else if (strcmp(what, "text") == 0) {
		int which = uiRadioButtonsSelected(uiRadioButtons(lobj->object)) + 1;
		if (which > 0) {
			if (lui_aux_getUservalue(L, 1, "values") != LUA_TNIL) {
				lua_rawgeti(L, -1, which);
			}
		}
	} else if (strcmp(what, "onselected") == 0) {
		lui_objectGetHandler(L, "onselected");
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_radiobuttons__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkRadiobuttons(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "selected") == 0) {
		int selected = luaL_checkinteger(L, 3) - 1;
		uiRadioButtonsSetSelected(uiRadioButtons(lobj->object), selected);
	} else if (strcmp(what, "onselected") == 0) {
		lui_objectSetHandler(L, "onselected", 3);
	} else {
		return lui_control__newindex(L);
	}
	return 0;
}

/*** Method
 * Object: radiobuttons
 * Name: append
 * Signature: radiobuttons:append(option, ...)
 * append one or more options to a radiobuttons control.
 */
static int lui_radiobuttonsAppend(lua_State *L)
{
	lui_object *lobj = lui_checkRadiobuttons(L, 1);
	const char *text = lua_tostring(L, 2);
	if (lui_aux_getUservalue(L, 1, "values") == LUA_TNIL) {
		lua_pop(L, 1);
		lua_newtable(L);
	}
	int tbl = lua_gettop(L);
	uiRadioButtonsAppend(uiRadioButtons(lobj->object), text);
	lui_aux_tinsert(L, tbl, -1, 2);
	for (int i = 3; i < tbl; ++i) {
		text = lua_tostring(L, i);
		uiRadioButtonsAppend(uiRadioButtons(lobj->object), text);
		lui_aux_tinsert(L, tbl, -1, i);
	}
	lui_aux_setUservalue(L, 1, "values", tbl);
	lua_pop(L, 1);
	return 0;
}

static void lui_radiobuttonsOnSelectedCallback(uiRadioButtons *spb, void *data)
{
	lua_State *L = (lua_State*) data;
	int top = lua_gettop(L);
	lui_objectHandlerCallback(L, uiControl(spb), "onselected", top, 0, 1);
	lua_settop(L, top);
}

/*** Constructor
 * Object: radiobuttons
 * Name: radiobuttons
 * Signature: radiobuttons = lui.radiobuttons()
 * create a radiobuttons control.
 */
static int lui_newRadiobuttons(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);
	lui_object *lobj = lui_pushRadiobuttons(L);
	lobj->object = uiNewRadioButtons();
	uiRadioButtonsOnSelected(uiRadioButtons(lobj->object), lui_radiobuttonsOnSelectedCallback, L);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for radiobuttons */
static const luaL_Reg lui_radiobuttons_meta[] = {
	{"__index", lui_radiobuttons__index},
	{"__newindex", lui_radiobuttons__newindex},
	{0, 0}
};

/* methods for radiobuttons */
static const luaL_Reg lui_radiobuttons_methods[] = {
	{"append", lui_radiobuttonsAppend},
	{0, 0}
};

/* datetime picker  ********************************************************/

/*** Object
 * Name: datetimepicker
 * a date / time picker control.
 */
#define LUI_DATETIMEPICKER "lui_datetimepicker"
#define lui_pushDatetimepicker(L) lui_pushObject(L, LUI_DATETIMEPICKER, 1)
#define lui_checkDatetimepicker(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_DATETIMEPICKER))

/*** Property
 * Object: datetimepicker
 * Name: day
 * the selected day, range from 1-31
 *** Property
 * Object: datetimepicker
 * Name: mon
 * the selected month, range from 1-12
 *** Property
 * Object: datetimepicker
 * Name: year
 * the selected year
 *** Property
 * Object: datetimepicker
 * Name: hour
 * the selected hour, range from 0-23
 *** Property
 * Object: datetimepicker
 * Name: min
 * the selected minute, range from 0-63
 *** Property
 * Object: datetimepicker
 * Name: sec
 * thee selected second, range from 0-63
 *** Property
 * Object: datetimepicker
 * Name: date
 * a table with fields day, mon, year
 *** Property
 * Object: datetimepicker
 * Name: time
 * a table with fields hour, min, sec
 *** Property
 * Object: datetimepicker
 * Name: datetime
 * a table with fields day, mon, year, hour, min, sec
 *** Property
 * Object: datetimepicker
 * Name: onchanged
 * a function <code>onchanged(datetimepicker)</code> that is called when the
 * contents of the datetimepicker is changed by the user.
 */
static int lui_dateTimePicker__index(lua_State *L)
{
	lui_object *lobj = lui_checkDatetimepicker(L, 1);
	const char *what = luaL_checkstring(L, 2);

	struct tm datetime;
	uiDateTimePickerTime(uiDateTimePicker(lobj->object), &datetime);

	if (strcmp(what, "day") == 0) {
		lua_pushinteger(L, datetime.tm_wday);
	} else if (strcmp(what, "mon") == 0) {
		lua_pushinteger(L, datetime.tm_mon + 1);
	} else if (strcmp(what, "year") == 0) {
		lua_pushinteger(L, datetime.tm_year + 1900);
	} else if (strcmp(what, "hour") == 0) {
		lua_pushinteger(L, datetime.tm_hour);
	} else if (strcmp(what, "min") == 0) {
		lua_pushinteger(L, datetime.tm_min);
	} else if (strcmp(what, "sec") == 0) {
		lua_pushinteger(L, datetime.tm_sec);
	} else if (strcmp(what, "date") == 0) {
		lua_newtable(L);
		lua_pushinteger(L, datetime.tm_wday);
		lua_setfield(L, -2, "day");
		lua_pushinteger(L, datetime.tm_mon + 1);
		lua_setfield(L, -2, "mon");
		lua_pushinteger(L, datetime.tm_year + 1900);
		lua_setfield(L, -2, "year");
	} else if (strcmp(what, "time") == 0) {
		lua_newtable(L);
		lua_pushinteger(L, datetime.tm_hour);
		lua_setfield(L, -2, "hour");
		lua_pushinteger(L, datetime.tm_min);
		lua_setfield(L, -2, "min");
		lua_pushinteger(L, datetime.tm_sec);
		lua_setfield(L, -2, "sec");
	} else if (strcmp(what, "datetime") == 0) {
		lua_newtable(L);
		lua_pushinteger(L, datetime.tm_wday);
		lua_setfield(L, -2, "day");
		lua_pushinteger(L, datetime.tm_mon + 1);
		lua_setfield(L, -2, "mon");
		lua_pushinteger(L, datetime.tm_year + 1900);
		lua_setfield(L, -2, "year");
		lua_pushinteger(L, datetime.tm_hour);
		lua_setfield(L, -2, "hour");
		lua_pushinteger(L, datetime.tm_min);
		lua_setfield(L, -2, "min");
		lua_pushinteger(L, datetime.tm_sec);
		lua_setfield(L, -2, "sec");
	} else if (strcmp(what, "onchanged") == 0) {
		lui_objectGetHandler(L, "onchanged");
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_dateTimePicker__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkDatetimepicker(L, 1);
	const char *what = luaL_checkstring(L, 2);

	struct tm datetime;
	uiDateTimePickerTime(uiDateTimePicker(lobj->object), &datetime);

	if (strcmp(what, "day") == 0) {
		datetime.tm_wday = luaL_checkinteger(L, 3);
	} else if (strcmp(what, "mon") == 0) {
		datetime.tm_mon = luaL_checkinteger(L, 3) - 1;
	} else if (strcmp(what, "year") == 0) {
		datetime.tm_year = luaL_checkinteger(L, 3) - 1900;
	} else if (strcmp(what, "hour") == 0) {
		datetime.tm_hour = luaL_checkinteger(L, 3);
	} else if (strcmp(what, "min") == 0) {
		datetime.tm_min = luaL_checkinteger(L, 3);
	} else if (strcmp(what, "sec") == 0) {
		datetime.tm_sec = luaL_checkinteger(L, 3);
	} else if (strcmp(what, "date") == 0) {
		luaL_checktype(L, 3, LUA_TTABLE);
		lua_getfield(L, 3, "day");
		datetime.tm_wday = luaL_checkinteger(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 3, "mon");
		datetime.tm_mon = luaL_checkinteger(L, -1) - 1;
		lua_pop(L, 1);
		lua_getfield(L, 3, "year");
		datetime.tm_year = luaL_checkinteger(L, -1) - 1900;
		lua_pop(L, 1);
	} else if (strcmp(what, "time") == 0) {
		luaL_checktype(L, 3, LUA_TTABLE);
		lua_getfield(L, 3, "hour");
		datetime.tm_hour = luaL_checkinteger(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 3, "min");
		datetime.tm_min = luaL_checkinteger(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 3, "sec");
		datetime.tm_sec = luaL_checkinteger(L, -1);
		lua_pop(L, 1);
	} else if (strcmp(what, "datetime") == 0) {
		luaL_checktype(L, 3, LUA_TTABLE);
		lua_getfield(L, 3, "day");
		datetime.tm_wday = luaL_checkinteger(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 3, "mon");
		datetime.tm_mon = luaL_checkinteger(L, -1) - 1;
		lua_pop(L, 1);
		lua_getfield(L, 3, "year");
		datetime.tm_year = luaL_checkinteger(L, -1) - 1900;
		lua_pop(L, 1);
		lua_getfield(L, 3, "hour");
		datetime.tm_hour = luaL_checkinteger(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 3, "min");
		datetime.tm_min = luaL_checkinteger(L, -1);
		lua_pop(L, 1);
		lua_getfield(L, 3, "sec");
		datetime.tm_sec = luaL_checkinteger(L, -1);
		lua_pop(L, 1);
	} else if (strcmp(what, "onchanged") == 0) {
		lui_objectSetHandler(L, "onchanged", 3);
	} else {
		return lui_control__newindex(L);
	}
	uiDateTimePickerSetTime(uiDateTimePicker(lobj->object), &datetime);
	return 0;
}

static void lui_dateTimePickerOnChangedCallback(uiDateTimePicker *dtp, void *data)
{
	lua_State *L = (lua_State*) data;
	int top = lua_gettop(L);
	lui_objectHandlerCallback(L, uiControl(dtp), "onchanged", top, 0, 1);
	lua_settop(L, top);
}

/*** Constructor
 * Object: datetimepicker
 * Name: datetimepicker
 * Signature: datetimepicker = lui.datetimepicker()
 */
static int lui_newDatetimepicker(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);
	lui_object *lobj = lui_pushDatetimepicker(L);
	lobj->object = uiNewDateTimePicker();
	uiDateTimePickerOnChanged(uiDateTimePicker(lobj->object), lui_dateTimePickerOnChangedCallback, L);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/*** Constructor
 * Object: datetimepicker
 * Name: datepicker
 * Signature: datetimepicker = lui.datepicker()
 */
static int lui_newDatepicker(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);
	lui_object *lobj = lui_pushDatetimepicker(L);
	lobj->object = uiNewDatePicker();
	uiDateTimePickerOnChanged(uiDateTimePicker(lobj->object), lui_dateTimePickerOnChangedCallback, L);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/*** Constructor
 * Object: datetimepicker
 * Name: timepicker
 * Signature: datetimepicker = lui.timepicker()
 */
static int lui_newTimepicker(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);
	lui_object *lobj = lui_pushDatetimepicker(L);
	lobj->object = uiNewTimePicker();
	uiDateTimePickerOnChanged(uiDateTimePicker(lobj->object), lui_dateTimePickerOnChangedCallback, L);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for dateTimePicker */
static const luaL_Reg lui_dateTimePicker_meta[] = {
	{"__index", lui_dateTimePicker__index},
	{"__newindex", lui_dateTimePicker__newindex},
	{0, 0}
};

/* multiline entry control  ************************************************/

/*** Object
 * Name: multilineentry
 * a multiline entry control
 */
#define LUI_MULTILINEENTRY "lui_multilineentry"
#define lui_pushMultilineEntry(L) lui_pushObject(L, LUI_MULTILINEENTRY, 1)
#define lui_checkMultilineEntry(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_MULTILINEENTRY))

/*** Property
 * Object: multilineentry
 * Name: text
 * the text the multiline entry contains.
 *** Property
 * Object: multilineentry
 * Name: readonly
 * true if the multiline entry is / should be read only, false if not.
 *** Property
 * Object: multilineentry
 * Name: onchanged
 * a function <code>onchanged(multilineentry)</code> that is called when the
 * contents of the multilineentry is changed by the user.
 */
static int lui_multilineEntry__index(lua_State *L)
{
	lui_object *lobj = lui_checkMultilineEntry(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "text") == 0) {
		char *text = uiMultilineEntryText(uiMultilineEntry(lobj->object));
		lua_pushstring(L, text);
		uiFreeText(text);
	} else if (strcmp(what, "readonly") == 0) {
		lua_pushboolean(L, uiMultilineEntryReadOnly(uiMultilineEntry(lobj->object)) != 0);
	} else if (strcmp(what, "onchanged") == 0) {
		lui_objectGetHandler(L, "onchanged");
	} else {
		return lui_control__index(L);
	}
	return 1;
}

static int lui_multilineEntry__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkMultilineEntry(L, 1);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "text") == 0) {
		const char *text = luaL_checkstring(L, 3);
		uiMultilineEntrySetText(uiMultilineEntry(lobj->object), text);
	} else if (strcmp(what, "readonly") == 0) {
		int readonly = lua_toboolean(L, 3);
		uiMultilineEntrySetReadOnly(uiMultilineEntry(lobj->object), readonly);
	} else if (strcmp(what, "onchanged") == 0) {
		lui_objectSetHandler(L, "onchanged", 3);
	} else {
		return lui_control__newindex(L);
	}
	return 0;
}

/*** Method
 * Object: multilineentry
 * Name: append
 * Signature: multilineentry:append(text, ...)
 * append one or more text pieces to the multilineentry control.
 */
static int lui_multilineEntryAppend(lua_State *L)
{
	lui_object *lobj = lui_checkMultilineEntry(L, 1);
	const char *text = lua_tostring(L, 2);
	uiMultilineEntryAppend(uiMultilineEntry(lobj->object), text);
	for (int i = 3; i <= lua_gettop(L); ++i) {
		text = lua_tostring(L, i);
		uiMultilineEntryAppend(uiMultilineEntry(lobj->object), text);
	}	return 0;
}

static void lui_multilineEntryOnChangedCallback(uiMultilineEntry *spb, void *data)
{
	lua_State *L = (lua_State*) data;
	int top = lua_gettop(L);
	lui_objectHandlerCallback(L, uiControl(spb), "onchanged", top, 0, 1);
	lua_settop(L, top);
}

/*** Constructor
 * Object: multilineentry
 * Name: multilineentry
 * Signature: multilineentry = lui.multilineentry(nowrapping = false, properties = nil)
 * create a new multilineentry control. if nowrapping is true, the text will
 * not wrap around when it exceeds the width of the control, but will be
 * accessible via  horizontal scrollbars instead.
 */
static int lui_newMultilineEntry(lua_State *L)
{
	int nowrapping = lua_toboolean(L, 1);
	int hastable = lui_aux_istable(L, 2);

	lui_object *lobj = lui_pushMultilineEntry(L);
	if (nowrapping) {
		lobj->object = uiNewNonWrappingMultilineEntry();
	} else {
		lobj->object = uiNewMultilineEntry();
	}
	uiMultilineEntryOnChanged(uiMultilineEntry(lobj->object), lui_multilineEntryOnChangedCallback, L);
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 2); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for multiline entry */
static const luaL_Reg lui_multilineEntry_meta[] = {
	{"__index", lui_multilineEntry__index},
	{"__newindex", lui_multilineEntry__newindex},
	{0, 0}
};

/* methods for multiline entry */
static const luaL_Reg lui_multilineEntry_methods[] = {
	{"append", lui_multilineEntryAppend},
	{0, 0}
};

/* controls function list table
 */
static const struct luaL_Reg lui_control_funcs [] ={
	{"button", lui_newButton},
	{"entry", lui_newEntry},
	{"passwordentry", lui_newPasswordEntry},
	{"searchentry", lui_newSearchEntry},
	{"checkbox", lui_newCheckbox},
	{"label", lui_newLabel},
	{"spinbox", lui_newSpinbox},
	{"progressbar", lui_newProgressbar},
	{"slider", lui_newSlider},
	{"hseparator", lui_newHSeparator},
	{"vseparator", lui_newVSeparator},
	{"combobox", lui_newCombobox},
	{"editablecombobox", lui_newEditableCombobox},
	{"radiobuttons", lui_newRadiobuttons},
	{"datetimepicker", lui_newDatetimepicker},
	{"datepicker", lui_newDatepicker},
	{"timepicker", lui_newTimepicker},
	{"multilineentry", lui_newMultilineEntry},
	{0, 0}
};

static int lui_init_controls(lua_State *L)
{
	luaL_setfuncs(L, lui_control_funcs, 0);

	lui_add_control_type(L, LUI_BUTTON, 0, lui_button_meta);
	lui_add_control_type(L, LUI_BOX, lui_box_methods, lui_box_meta);
	lui_add_control_type(L, LUI_ENTRY, 0, lui_entry_meta);
	lui_add_control_type(L, LUI_CHECKBOX, 0, lui_checkbox_meta);
	lui_add_control_type(L, LUI_LABEL, 0, lui_label_meta);
	lui_add_control_type(L, LUI_SPINBOX, 0, lui_spinbox_meta);
	lui_add_control_type(L, LUI_PROGRESSBAR, 0, lui_progressbar_meta);
	lui_add_control_type(L, LUI_SLIDER, 0, lui_slider_meta);
	lui_add_control_type(L, LUI_SEPARATOR, 0, 0);
	lui_add_control_type(L, LUI_COMBOBOX, lui_combobox_methods, lui_combobox_meta);
	lui_add_control_type(L, LUI_EDITABLECOMBOBOX, lui_editableCombobox_methods, lui_editableCombobox_meta);
	lui_add_control_type(L, LUI_RADIOBUTTONS, lui_radiobuttons_methods, lui_radiobuttons_meta);
	lui_add_control_type(L, LUI_DATETIMEPICKER, 0, lui_dateTimePicker_meta);
	lui_add_control_type(L, LUI_MULTILINEENTRY, lui_multilineEntry_methods, lui_multilineEntry_meta);

	return 1;
}
