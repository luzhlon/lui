/* uiTableModel *************************************************************/

/*** Object
 * Name: tablemodel
 * a tablemodel provided the data to a table. Communication happens through
 * callback functions.
 */

/* TODO L should really be a void* (userdata) associated with the model,
 * instead of the modelhandler. Check if (when) this will be implemented
 * in libui */
struct myUiTableModelHandler {
	struct uiTableModelHandler handler;
	lua_State *L;
};

#define uiTableModel(this) ((uiTableModel *) (this))
#define LUI_TABLEMODEL "lui_tablemodel"
#define lui_pushTableModel(L) lui_pushObject(L, LUI_TABLEMODEL, 1)
#define lui_checkTableModel(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_TABLEMODEL))

#define LUI_TABLEMODEL_REGISTRY "lui_tablemodel_registry"

static int lui_tablemodel__gc(lua_State *L)
{
	lui_object *lobj = lui_checkTableModel(L, 1);
	if (lobj->object) {
		DEBUGMSG("lui_tablemodel__gc (%s)", lui_debug_controlTostring(L, 1));
		uiFreeTableModel(uiTableModel(lobj->object));
		lobj->object = 0;
	}
	return 0;
}

/* metamethods for tablemodel */
static const luaL_Reg lui_tablemodel_meta[] = {
	{"__gc", lui_tablemodel__gc},
	{0, 0}
};

/*** Method
 * Object: tablemodel
 * Name: row_inserted
 * Signature: mdl:row_inserted(rownumber)
 * signals to any connected table that a row has been inserted into the data
 * represented by the table model.
 */
static int lui_tablemodel_row_inserted(lua_State *L)
{
	lui_object *lobj = lui_checkTableModel(L, 1);
	int row = lua_tointeger(L, 2);
	uiTableModelRowInserted(uiTableModel(lobj->object), row);
	return 0;
}

/*** Method
 * Object: tablemodel
 * Name: row_changed
 * Signature: mdl:row_changed(rownumber)
 * signals to any connected table that a row has been changed in the data
 * represented by the table model. You need to only call this when the data
 * is changed by the program, if it has been changed by editing a cell in
 * the table control, you do not need to signal this to the table.
 */
static int lui_tablemodel_row_changed(lua_State *L)
{
	lui_object *lobj = lui_checkTableModel(L, 1);
	int row = lua_tointeger(L, 2);
	uiTableModelRowChanged(uiTableModel(lobj->object), row);
	return 0;
}

/*** Method
 * Object: tablemodel
 * Name: row_deleted
 * Signature: mdl:row_deleted(rownumber)
 * signals to any connected table that a row has been deleted from the data
 * represented by the table model.
 */
static int lui_tablemodel_row_deleted(lua_State *L)
{
	lui_object *lobj = lui_checkTableModel(L, 1);
	int row = lua_tointeger(L, 2);
	uiTableModelRowDeleted(uiTableModel(lobj->object), row);
	return 0;
}

/* methods for tablemodel */
static const luaL_Reg lui_tablemodel_methods[] = {
	{"row_inserted", lui_tablemodel_row_inserted},
	{"row_changed", lui_tablemodel_row_changed},
	{"row_deleted", lui_tablemodel_row_deleted},
	{0, 0}
};

/* lui control registry handling
 */
static int lui_registerTableModel(lua_State *L, int pos)
{
	lui_object *lobj = lui_toObject(L, pos);
	lua_getfield(L, LUA_REGISTRYINDEX, LUI_TABLEMODEL_REGISTRY);
	lua_pushlightuserdata(L, lobj->object);
	lua_pushvalue(L, pos);
	lua_settable(L, -3);
	lua_pop(L, 1);
	return 0;
}

static int lui_findTableModel(lua_State *L, const uiTableModel *tm)
{
	lua_getfield(L, LUA_REGISTRYINDEX, LUI_TABLEMODEL_REGISTRY);
	lua_pushlightuserdata(L, (uiTableModel*)tm);
	lua_gettable(L, -2);
	lua_replace(L, -2);
	return lua_type(L, -1);
}

static int lui_findhandler(lua_State *L, uiTableModel *tm, const char *func)
{
	if (lui_findTableModel(L, tm) == LUA_TNIL) {
		lua_pop(L, 1);
		return 0; // TODO should this be a lua error?
	}
	if (lui_aux_getUservalue(L, -1, "handler") == LUA_TTABLE) {
		lua_getfield(L, -1, func);
		lua_replace(L, -3);
		lua_pop(L, 1);
		return 1;
	}
	lua_pop(L, 2);
	return 0;
}

static int lui_tablemodelhandler_numcolumns(uiTableModelHandler *tmh, uiTableModel *tm)
{
	lua_State *L = ((struct myUiTableModelHandler*)tmh)->L;
	if (lui_findhandler(L, tm, "numcolumns")) {
		lua_call(L, 0, 1);
		int res = lua_tointeger(L, -1);
		lua_pop(L, 1);
		return res;
	}
	return 0;
}

static int lui_tablemodelhandler_numrows(uiTableModelHandler *tmh, uiTableModel *tm)
{
	lua_State *L = ((struct myUiTableModelHandler*)tmh)->L;
	if (lui_findhandler(L, tm, "numrows")) {
		lua_call(L, 0, 1);
		int res = lua_tointeger(L, -1);
		lua_pop(L, 1);
		return res;
	}
	return 0;
}

typedef enum {
	lui_TableValueTypeNull,
	lui_TableValueTypeString,
	lui_TableValueTypeInt,
	lui_TableValueTypeBool,
	lui_TableValueTypeImage,
	lui_TableValueTypeColor,
} lui_TableValueType;

static lui_TableValueType lui_tablemodelhandler_rawcolumntype(uiTableModelHandler *tmh, uiTableModel *tm, int col)
{
	DEBUGMSG("lui_tablemodelhandler_rawcolumntype called for col %d", col);
	uiTableValueType res = lui_TableValueTypeNull;
	lua_State *L = ((struct myUiTableModelHandler*)tmh)->L;
	if (lui_findTableModel(L, tm) != LUA_TNIL) {
		if (lui_aux_getUservalue(L, -1, "columntype") != LUA_TTABLE) {
			lua_pop(L, 1);
			lua_newtable(L);
			lui_aux_setUservalue(L, -2, "columntype", -1);
		}
		if (lua_rawgeti(L, -1, col) != LUA_TNIL) {
			res = lua_tointeger(L, -1);
			lua_pop(L, 3);
			return res;
		}
		lua_pop(L, 1); // tm uv

		if (lui_findhandler(L, tm, "columntype")) {
			lua_pushinteger(L, col);
			lua_call(L, 1, 1);
			const char *type = lua_tostring(L, -1);
			if (!type || !strcmp(type, "string")) {
				res = lui_TableValueTypeString;
			} else if (!strcmp(type, "int") || !strcmp(type, "integer")) {
				res = lui_TableValueTypeInt;
			} else if (!strcmp(type, "bool") || !strcmp(type, "boolean")) {
				res = lui_TableValueTypeBool;
			} else if (!strcmp(type, "image")) {
				res = lui_TableValueTypeImage;
			} else if (!strcmp(type, "color")) {
				res = lui_TableValueTypeColor;
			} else {
				puts("Error!\n");//TODO error
			}
			lua_pop(L, 1);
		}

		lua_pushinteger(L, res);
		lua_rawseti(L, -2, col);

		lua_pop(L, 2);
	}
	return res;
}

static uiTableValueType lui_tablemodelhandler_columntype(uiTableModelHandler *tmh, uiTableModel *tm, int col)
{
	DEBUGMSG("lui_tablemodelhandler_columntype called for col %d", col);
	lui_TableValueType raw = lui_tablemodelhandler_rawcolumntype(tmh, tm, col);
	uiTableValueType res = uiTableValueTypeString;
	switch (raw) {
		case lui_TableValueTypeNull:
		case lui_TableValueTypeString:
			res = uiTableValueTypeString;
			break;
		case lui_TableValueTypeInt:
		case lui_TableValueTypeBool:
			res = uiTableValueTypeInt;
			break;
		case lui_TableValueTypeImage:
			res = uiTableValueTypeImage;
			break;
		case lui_TableValueTypeColor:
			res = uiTableValueTypeColor;
			break;
	}
	return res;
}

static uiTableValue *lui_tablemodel_totablevaluestring(lua_State *L, int pos)
{
	if (pos < 0) {
		pos = lua_gettop(L) + 1 + pos;
	}
	lua_getglobal(L, "tostring");
	lua_pushvalue(L, -2);
	lua_call(L, 1, 1);
	uiTableValue *res = uiNewTableValueString(lua_tostring(L, -1));
	lua_pop(L, 1);
	return res;
}

static uiTableValue *lui_tablemodelhandler_cellvalue(uiTableModelHandler *tmh, uiTableModel *tm, int row, int col)
{
	DEBUGMSG("lui_tablemodelhandler_cellvalue called for row %d col %d", row, col);
	uiTableValue *res = NULL;
	lua_State *L = ((struct myUiTableModelHandler*)tmh)->L;
	if (lui_findhandler(L, tm, "cellvalue")) {
		lua_pushinteger(L, row + 1);
		lua_pushinteger(L, col);
		lua_call(L, 2, 1);

		uiTableValueType vtype = lui_tablemodelhandler_columntype(tmh, tm, col);

		switch (lua_type(L, -1)) {
			case LUA_TBOOLEAN:
				if (vtype == uiTableValueTypeInt) {
					res = uiNewTableValueInt(lua_toboolean(L, -1));
				} else if (vtype == uiTableValueTypeString) {
					res = lui_tablemodel_totablevaluestring(L, -1);
				} else {
					puts("Error!\n");// TODO error
				}
				break;
			case LUA_TNUMBER:
				if (vtype == uiTableValueTypeInt) {
					res = uiNewTableValueInt(lua_tointeger(L, -1));
				} else if (vtype == uiTableValueTypeString) {
					res = uiNewTableValueString(lua_tostring(L, -1));
				} else {
					puts("Error!\n");// TODO error
				}
				break;
			case LUA_TTABLE:
				if (vtype == uiTableValueTypeColor) {
					double r, g, b, a;
					lui_aux_rgbaFromTable(L, -1, &r, &g, &b, &a);
					res = uiNewTableValueColor(r, g, b, a);
				} else if (vtype == uiTableValueTypeString) {
					res = lui_tablemodel_totablevaluestring(L, -1);
				} else {
					puts("Error!\n");// TODO error
				}
				break;
			case LUA_TUSERDATA:
				if (vtype == uiTableValueTypeImage) {
					lui_object *lobj = lui_checkImage(L, -1);
					res = uiNewTableValueImage(lobj->object);
				} else if (vtype == uiTableValueTypeString) {
					res = lui_tablemodel_totablevaluestring(L, -1);
				} else {
					puts("Error!\n");// TODO error
				}
				break;
			case LUA_TSTRING:
			default:
				if (vtype == uiTableValueTypeString) {
					res = lui_tablemodel_totablevaluestring(L, -1);
				} else {
					puts("Error!\n");// TODO error
				}
				break;
		}

		if (!res) {
			// TODO error
			uiNewTableValueString("Oh Boy 1...");
		}

		lua_pop(L, 1);
		return res;
	}
	if (!res) {
		// TODO error
		uiNewTableValueString("Oh Boy 2...");
	}
	return res;
}

static void lui_tablemodelhandler_setcellvalue(uiTableModelHandler *tmh, uiTableModel *tm, int row, int col, const uiTableValue *tv)
{
	DEBUGMSG("lui_tablemodelhandler_setcellvalue called for row %d col %d", row, col);
	lua_State *L = ((struct myUiTableModelHandler*)tmh)->L;
	if (lui_findhandler(L, tm, "setcellvalue")) {
		lua_pushinteger(L, row + 1);
		lua_pushinteger(L, col);
		if (tv) {
			lui_TableValueType rtype = lui_tablemodelhandler_rawcolumntype(tmh, tm, col);
			switch (uiTableValueGetType(tv)) {
				case uiTableValueTypeString:
				if (rtype == lui_TableValueTypeNull || rtype == lui_TableValueTypeString) {
						lua_pushstring(L, uiTableValueString(tv));
					} else {
						//TODO error
						lua_pushnil(L);
					}
					break;
				case uiTableValueTypeInt:
					if (rtype == lui_TableValueTypeInt) {
						lua_pushinteger(L, uiTableValueInt(tv));
					} else if (rtype == lui_TableValueTypeBool) {
						lua_pushboolean(L, uiTableValueInt(tv));
					} else {
						//TODO error
						lua_pushnil(L);
					}
					break;
				default:
					lua_pushnil(L); // this should not happen
			}
		} else {
			lua_pushnil(L);
		}
		lua_call(L, 3, 0);
	}
}

static int lui_checkHandlerTable(lua_State *L, int pos)
{
	lua_pushnil(L);
	while (lua_next(L, pos) != 0) {
		const char *name = lua_tostring(L, -2);
		int vtype = lua_type(L, -1);
		if (strcmp(name, "numcolumns") != 0 &&
			strcmp(name, "columntype") != 0 &&
			strcmp(name, "numrows") != 0 &&
			strcmp(name, "cellvalue") != 0 &&
			strcmp(name, "setcellvalue") != 0) {
			return luaL_error(L, "invalid field in handler table");
		}

		if (vtype != LUA_TFUNCTION) {
			return luaL_error(L, "invalid value in handdler table");
		}

		lua_pop(L, 1);
	}
	return 1;
}

/*** Constructor
 * Object: tablemodel
 * Name: tablemodel
 * signature: mdl = lui.tablemodel( handlerfuncs )
 * creates a new table model. handlerfuncs is a table with 5 named entries,
 * which are user supplied functions the table uses to get and set data from
 * the model. These functions are:
 *
 *	numcolumns()
 * 		must return the number of columns in the data
 *	numrows()
 *		must return the number of rows in the data
 *	columntype(col)
 *		must return the type of data in column col. Valid return values are
 *		string, integer, boolean, image and color. All values that have a
 *		textual representation in a cell must be returned as string, the
 *		other types are only valid for special column types. Note that the
 *		type for a column must remain fixed throughout the tabemoodels life
 *		time. Your function is most probably only called once.
 *	cellvalue(row, col)
 *		must return the data in row, col. The data must correspond to the
 *		type returned by columntype for this column, except in the case of
 *		string. Anything will be converted to it's string representation
 *		for string type columns.
 *	setcellvalue(row, col, val)
 *		must set the data in row, col to value val.
 */
static int lui_newTableModel(lua_State *L)
{
	struct myUiTableModelHandler *tmh = calloc(1, sizeof(struct myUiTableModelHandler));
	tmh->handler.NumColumns = lui_tablemodelhandler_numcolumns;
	tmh->handler.ColumnType = lui_tablemodelhandler_columntype;
	tmh->handler.NumRows = lui_tablemodelhandler_numrows;
	tmh->handler.CellValue = lui_tablemodelhandler_cellvalue;
	tmh->handler.SetCellValue = lui_tablemodelhandler_setcellvalue;
	tmh->L = L;
	// TODO does the model own the modelhandler? should probably handle with __gc?

	lui_checkHandlerTable(L, 1);
	lui_object *lobj = lui_pushTableModel(L);
	lobj->object = uiNewTableModel((uiTableModelHandler *)tmh);

	lua_getuservalue(L, -1);
	// TODO what about tmh?
	lua_pushvalue(L, 1);
	lua_setfield(L, -2, "handler");
	lua_pop(L, 1);

	lui_registerTableModel(L, lua_gettop(L));

	return  1;
}

/* uiTable ******************************************************************/

/*** Object
 * Name: table
 */

#define uiTable(this) ((uiTable *) (this))
#define LUI_TABLE "lui_table"
#define lui_pushTable(L) lui_pushObject(L, LUI_TABLE, 1)
#define lui_checkTable(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_TABLE))

/*** Method
 * Object: table
 * Name: appendtextcolumn
 * Signature: tbl:appendtextcolumn(name, datacolumn, editable = false, textcolorcolumn = nil)
 * appends a text column to a table. name is displayed as the column header.
 * datacolumn is the column from the underlying datamodel, where the text
 * comes from. If a row is editable according to the editable parameter,
 * SetCellValue() is called with datacolumn as the column. If the
 * textcolorcolumn argument is ~= nil, it is a column that must hold a
 * color value which is to be used for the text color for this column.
 */
static int lui_table_appendtextcolumn(lua_State *L)
{
	lui_object *lobj = lui_checkTable(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int column = luaL_checkinteger(L, 3);
	int editable = lua_toboolean(L, 4) ? uiTableModelColumnAlwaysEditable : uiTableModelColumnNeverEditable;
	int cmc = luaL_optinteger(L, 5, -1);
	uiTableTextColumnOptionalParams tparm = { .ColorModelColumn = cmc };
	uiTableAppendTextColumn(uiTable(lobj->object), name, column, editable, &tparm);
	return 0;
}

/*** Method
 * Object: table
 * Name: appendimagecolumn
 * Signature: tbl:appendimagecolumn(name, datacolumn)
 * appends an image column to a table. name is displayed as the column
 * header. datacolumn is the column from the data model that must hold an
 * image object to be displayed in this column.
 */
static int lui_table_appendimagecolumn(lua_State *L)
{
	lui_object *lobj = lui_checkTable(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int column = luaL_checkinteger(L, 3);
	uiTableAppendImageColumn(uiTable(lobj->object), name, column);
	return 0;
}

/*** Method
 * Object: table
 * Name: appendimagetextcolumn
 * Signature: tbl:appendimagetextcolumn(name, icolumn, tcolumn, editable = false, textcolorcolumn = nil)
 * appends a column to a table that displays both an image and a text. 
 * name is displayed as the column header. icolumn is the column from the
 * datamodel that must hold an image object to be displayed in this column.
 * tcolumn  is the column from the underlying datamodel, where the text
 * comes from. If a row is editable according to the editable parameter,
 * SetCellValue() is called with datacolumn as the column. If the
 * textcolorcolumn argument is ~= nil, it is a column that must hold a
 * color value which is to be used for the text color for this column.
 */
static int lui_table_appendimagetextcolumn(lua_State *L)
{
	lui_object *lobj = lui_checkTable(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int icolumn = luaL_checkinteger(L, 3);
	int tcolumn = luaL_checkinteger(L, 4);
	int editable = lua_toboolean(L, 5) ? uiTableModelColumnAlwaysEditable : uiTableModelColumnNeverEditable;
	int cmc = luaL_optinteger(L, 6, -1);
	uiTableTextColumnOptionalParams tparm = { .ColorModelColumn = cmc };
	uiTableAppendImageTextColumn(uiTable(lobj->object), name, icolumn, tcolumn, editable, &tparm);
	return 0;
}

/*** Method
 * Object: table
 * Name: appendcheckboxcolumn
 * Signature: tbl:appendcheckboxcolumn(name, datacolumn, editable)
 * appends a column that contains a checkbox. name is displayed as the
 * column header. datacolumn is the column from the datamodel that holds the
 * data for this checkbox. The data in this column must be either an integer
 * with values 0 or 1, or a boolean. If editable is true, this checkbox can
 * be interacted with, and setcellvalue() will be called with datamodelcolumn
 * as the column in this case.
 */
static int lui_table_appendcheckboxcolumn(lua_State *L)
{
	lui_object *lobj = lui_checkTable(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int column = luaL_checkinteger(L, 3);
	int editable = lua_toboolean(L, 4) ? uiTableModelColumnAlwaysEditable : uiTableModelColumnNeverEditable;
	uiTableAppendCheckboxColumn(uiTable(lobj->object), name, column, editable);
	return 0;
}

/*** Method
 * Object: table
 * Name: appendcheckboxtextcolumn
 * Signature: tbl:appendcheckboxtextcolumn(name, ccolumn, ceditable, tcolumn, teditable = false, textcolorcolumn = nil)
 * appends a column that contains both a checkbox and text. ccolumn is
 * the data column for the checkbox, ceditable is the flag whether the
 * checkbox can be interacted with. tcolumn and teditable are the same for
 * the text, and textcolurcolumn is the optional column for the text
 * foreground color. The checkbox arguments work as they do for the
 * appendcheckboxcolumn() method, the text arguments work as they do for the
 * appendtextcolumn() method.
 */
static int lui_table_appendcheckboxtextcolumn(lua_State *L)
{
	lui_object *lobj = lui_checkTable(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int ccolumn = luaL_checkinteger(L, 3);
	int ceditable = lua_toboolean(L, 4) ? uiTableModelColumnAlwaysEditable : uiTableModelColumnNeverEditable;
	int tcolumn = luaL_checkinteger(L, 5);
	int teditable = lua_toboolean(L, 6) ? uiTableModelColumnAlwaysEditable : uiTableModelColumnNeverEditable;
	int cmc = luaL_optinteger(L, 7, -1);
	uiTableTextColumnOptionalParams tparm = { .ColorModelColumn = cmc };
	uiTableAppendCheckboxTextColumn(uiTable(lobj->object), name, ccolumn, ceditable, tcolumn, teditable, &tparm);
	return 0;
}

/*** Method
 * Object: table
 * Name: appendprogressbarcolumn
 * Signature: tbl:appendprogressbarcolumn(name, column)
 * appends a column that displays a progress bar. These columns work
 * like lui.progressBar: a cell value of 0..100 displays that percentage,
 * and a cell value of -1 displays an indeterminate progress bar. The data
 * type to use for this column is integer.
 */
static int lui_table_appendprogressbarcolumn(lua_State *L)
{
	lui_object *lobj = lui_checkTable(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int column = luaL_checkinteger(L, 3);
	uiTableAppendProgressBarColumn(uiTable(lobj->object), name, column);
	return 0;
}

/*** Method
 * Object: table
 * Name: appendbuttoncolumn
 * Signature: tbl:appendbuttoncolumn(name, datacolumn, clickable = false)
 * appends a column that shows a button that the user can click on. When
 * the user does click on the button, SetCellValue() is called with a nil
 * value and datacolumn as the column. cellvalue() on datacolumn should
 * return the text to show in the button.
 */
static int lui_table_appendbuttoncolumn(lua_State *L)
{
	lui_object *lobj = lui_checkTable(L, 1);
	const char *name = luaL_checkstring(L, 2);
	int column = luaL_checkinteger(L, 3);
	int clickable = lua_toboolean(L, 4) ? uiTableModelColumnAlwaysEditable : uiTableModelColumnNeverEditable;
	uiTableAppendButtonColumn(uiTable(lobj->object), name, column, clickable);
	return 0;
}

/* methods for table */
static const luaL_Reg lui_table_methods[] = {
	{"appendtextcolumn", lui_table_appendtextcolumn},
	{"appendimagecolumn", lui_table_appendimagecolumn},
	{"appendimagetextcolumn", lui_table_appendimagetextcolumn},
	{"appendcheckboxcolumn", lui_table_appendcheckboxcolumn},
	{"appendcheckboxtextcolumn", lui_table_appendcheckboxtextcolumn},
	{"appendprogressbarcolumn", lui_table_appendprogressbarcolumn},
	{"appendbuttoncolumn", lui_table_appendbuttoncolumn},
	{0, 0}
};

/*** Constructor
 * Object: table
 * Name: table
 * TODO
 */
static int lui_newTable(lua_State *L)
{
	lui_object *lmodel = lui_checkTableModel(L, 1);
	int rbgcmc = luaL_optinteger(L, 2, -1);
	struct uiTableParams tparms = { .Model = lmodel->object, .RowBackgroundColorModelColumn = rbgcmc };

	lui_object *lobj = lui_pushTable(L);
	lobj->object = uiNewTable(&tparms);
	lui_aux_setUservalue(L, -1, "model", 1);

	return 1;
}

static const struct luaL_Reg lui_table_funcs [] ={
	/* utility constructors */
	{"tablemodel", lui_newTableModel},
	{"table", lui_newTable},
	{0, 0}
};

static int lui_init_table(lua_State *L)
{
	luaL_setfuncs(L, lui_table_funcs, 0);

	lui_add_utility_type(L, LUI_TABLEMODEL, lui_tablemodel_methods, lui_tablemodel_meta);
	lui_add_control_type(L, LUI_TABLE, lui_table_methods, NULL);

	/* create tablemodel registry */
	lua_newtable(L);
	lua_newtable(L);
	lua_pushstring(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
	lua_setfield(L, LUA_REGISTRYINDEX, LUI_TABLEMODEL_REGISTRY);

	return 1;
}
