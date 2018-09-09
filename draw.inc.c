/* lui.c
 *
 * lua binding to libui (https://github.com/andlabs/libui)
 *
 * Gunnar Zötl <gz@tset.de>, 2016
 * Released under MIT/X11 license. See file LICENSE for details.
 *
 * This file is included by lui.c
 */

/* uiDrawBrush  ************************************************************/

/*** Object
 * Name: draw.brush
 * a brush spec to draw with.
 */
#define uiDrawBrush(this) ((uiDrawBrush *) (this))
#define LUI_DRAWBRUSH "lui_drawbrush"
#define lui_pushDrawBrush(L) lui_pushObject(L, LUI_DRAWBRUSH, 1)
#define lui_checkDrawBrush(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_DRAWBRUSH))

static int lui_drawbrush_setGradientStops(lua_State *L, uiDrawBrush *brush, int pos)
{
	if (lua_isnoneornil(L, pos)) {
		if (brush->Stops) {
			free(brush->Stops);
		}
		brush->Stops = 0;
		brush->NumStops = 0;
		return 1;
	}
	if (lua_type(L, pos) != LUA_TTABLE) {
		return luaL_error(L, "invalid value for gradientstops (expected table)");
	}
	int len = lua_rawlen(L, pos);
	if (brush->Stops) {
		free(brush->Stops);
	}
	brush->Stops = calloc(len, sizeof(uiDrawBrushGradientStop));
	if (!brush->Stops) {
		luaL_error(L, "out of memory!");
	}
	brush->NumStops = len;
	for (int i = 0; i < len; ++i) {
		uiDrawBrushGradientStop *stop = &brush->Stops[i];
		lua_geti(L, pos, i + 1);
		if (lua_type(L, lua_gettop(L)) != LUA_TTABLE) {
			return luaL_error(L, "invalid value in table for gradientstops");
		}
		lui_aux_rgbaFromTable(L, lua_gettop(L), &stop->R, &stop->G, &stop->B, &stop->A);
		lua_geti(L, -1, 1);
		stop->Pos = luaL_checknumber(L, -1);
		lua_pop(L, 2);
	}
	return 1;
}

static int lui_drawbrush_getGradientStops(lua_State *L, uiDrawBrush *brush)
{
	if (brush->NumStops == 0) {
		lua_pushnil(L);
		return 1;
	}
	lua_newtable(L);
	for (int i = 0; i < brush->NumStops; ++i) {
		uiDrawBrushGradientStop *stop = &brush->Stops[i];
		lui_aux_pushRgbaAsTable(L, stop->R, stop->G, stop->B, stop->A);
		lua_pushnumber(L, stop->Pos);
		lua_rawseti(L, -2, 1);
		lua_seti(L, -2, i + 1);
	}
	return 1;
}

/*** Property
 * Object: draw.brush
 * Name: type
 * may be any of the strings "solid", "lineargradient", "radialgradient", "image"
 * or lui.enum.brushtype.solid ..., will always return type name on read.
 *** Property
 * Object: draw.brush
 * Name: color
 * color of the brush
 *** Property
 * Object: draw.brush
 * Name: x0
 * linear: start X, radial: start X
 *** Property
 * Object: draw.brush
 * Name: y0
 * linear: start Y, radial: start Y
 *** Property
 * Object: draw.brush
 * Name: x1
 * linear: end X, radial: outer circle center X
 *** Property
 * Object: draw.brush
 * Name: y1
 * linear: end Y, radial: outer circle center Y
 *** Property
 * Object: draw.brush
 * Name: outerradius
 * radial gradients only
 *** Property
 * Object: draw.brush
 * Name: gradientstops
 * { { pos, r=?, g=?, b=?, a=?}, ... }, don't modify what you read
 */
static int lui_drawbrush__index(lua_State *L)
{
	lui_object *lobj = lui_checkDrawBrush(L, 1);
	uiDrawBrush *brush = uiDrawBrush(lobj->object);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "type") == 0) {
		lui_aux_pushNameOrValue(L, brush->Type, "lui_enumbrushtype");
	} else if (strcmp(what, "color") == 0) {
		lui_aux_pushRgbaAsTable(L, brush->R, brush->A, brush->B, brush->A);
	} else if (strcmp(what, "x0") == 0) {
		lua_pushnumber(L, brush->X0);
	} else if (strcmp(what, "y0") == 0) {
		lua_pushnumber(L, brush->Y0);
	} else if (strcmp(what, "x1") == 0) {
		lua_pushnumber(L, brush->X1);
	} else if (strcmp(what, "y1") == 0) {
		lua_pushnumber(L, brush->Y1);
	} else if (strcmp(what, "outerradius") == 0) {
		lua_pushnumber(L, brush->OuterRadius);
	} else if (strcmp(what, "gradientstops") == 0) {
		if (lui_aux_getUservalue(L, 1, "gradientstops") == LUA_TNIL) {
			DEBUGMSG("(uncached)");
			lua_pop(L, 1);
			lui_drawbrush_getGradientStops(L, brush);
			lui_aux_setUservalue(L, 1, "gradientstops", 3);
		}
	} else {
		return lui_utility__index(L);
	}
	return 1;
}

static int lui_drawbrush__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkDrawBrush(L, 1);
	uiDrawBrush *brush = uiDrawBrush(lobj->object);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "type") == 0) {
		brush->Type = lui_aux_getNumberOrValue(L, 3, "lui_enumbrushtype");
	} else if (strcmp(what, "color") == 0) {
		if (lua_type(L, 3) != LUA_TTABLE) {
			return luaL_error(L, "invalid value for gradientstops");
		}
		lui_aux_rgbaFromTable(L, 3, &brush->R, &brush->G, &brush->B, &brush->A);
	} else if (strcmp(what, "x0") == 0) {
		brush->X0 = lua_tonumber(L, 3);
	} else if (strcmp(what, "y0") == 0) {
		brush->Y0 = lua_tonumber(L, 3);
	} else if (strcmp(what, "x1") == 0) {
		brush->X1 = lua_tonumber(L, 3);
	} else if (strcmp(what, "y1") == 0) {
		brush->Y1 = lua_tonumber(L, 3);
	} else if (strcmp(what, "outerradius") == 0) {
		brush->OuterRadius = lua_tonumber(L, 3);
	} else if (strcmp(what, "gradientstops") == 0) {
		lui_drawbrush_setGradientStops(L, brush, 3);
		lua_pushnil(L);
		lui_aux_setUservalue(L, 1, "gradientstops", 4);
		lua_pop(L, 1);
	} else {
		return lui_utility__newindex(L);
	}
	return 0;
}

static int lui_makeDrawBrushTypeEnum(lua_State *L)
{
	lua_newtable(L);
	lui_enumItem("solid", uiDrawBrushTypeSolid);
	lui_enumItem("lineargradient", uiDrawBrushTypeLinearGradient);
	lui_enumItem("radialgradient", uiDrawBrushTypeRadialGradient);
	lui_enumItem("image", uiDrawBrushTypeImage);
	return 1;
}

/*** Constructor
 * Object: draw.brush
 * Name: draw.brush
 * Signature: brush = lui.draw.brush(properties = nil)
 * create a new draw.brush object
 */
static int lui_newDrawBrush(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);

	lui_object *lobj = lui_pushDrawBrush(L);
	lobj->object = calloc(1, sizeof(uiDrawBrush));
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for draw.brush */
static const luaL_Reg lui_drawbrush_meta[] = {
	{"__index", lui_drawbrush__index},
	{"__newindex", lui_drawbrush__newindex},
	{0, 0}
};

/* uiDrawStrokeParams  *****************************************************/

/*** Object
 * Name: draw.strokeparams
 * a stroke spec to draw with
 */
#define uiDrawStrokeParams(this) ((uiDrawStrokeParams *) (this))
#define LUI_DRAWSTROKEPARAMS "lui_drawstrokeparams"
#define lui_pushDrawStrokeParams(L) lui_pushObject(L, LUI_DRAWSTROKEPARAMS, 1)
#define lui_checkDrawStrokeParams(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_DRAWSTROKEPARAMS))

static int lui_drawstrokeparams_setDashes(lua_State *L, uiDrawStrokeParams *params, int pos)
{
	if (lua_isnoneornil(L, pos)) {
		if (params->Dashes) {
			free(params->Dashes);
		}
		params->Dashes = 0;
		params->NumDashes = 0;
		return 1;
	}
	luaL_checktype(L, pos, LUA_TTABLE);
	int len = lua_rawlen(L, pos);
	if (params->Dashes) {
		free(params->Dashes);
	}
	params->Dashes = calloc(len, sizeof(double));
	if (!params->Dashes) {
		luaL_error(L, "out of memory!");
	}
	params->NumDashes = len;
	for (int i = 0; i < len; ++i) {
		lua_geti(L, pos, i + 1);
		params->Dashes[i] = luaL_checknumber(L, -1);
		lua_pop(L, 1);
	}
	return 1;

}

static int lui_drawstrokeparams_getDashes(lua_State *L, uiDrawStrokeParams *params)
{
	if (params->NumDashes == 0) {
		lua_pushnil(L);
		return 1;
	}
	lua_newtable(L);
	for (int i = 0; i < params->NumDashes; ++i) {
		lua_pushnumber(L, params->Dashes[i]);
		lua_seti(L, -2, i + 1);
	}
	return 1;
}

/*** Property
 * Object: draw.strokeparams
 * Name: linecap
 * any of the strings "flat", "round", "square", or lui.enum.linecap.flat...
 *** Property
 * Object: draw.strokeparams
 * Name: linejoin
 * any of the strings "miter", "round", "bevel", or lui.enum.linejoin.miter ...
 *** Property
 * Object: draw.strokeparams
 * Name: thickness
 * line thickness. Default is 1.
 *** Property
 * Object: draw.strokeparams
 * Name: miterlimit
 * line join miter limit. Default is 10.
 *** Property
 * Object: draw.strokeparams
 * Name: dashphase
 * (TODO doc missing)
 *** Property
 * Object: draw.strokeparams
 * Name: dashes
 *	{ len1, len2, ... }, don't modify what you read
 * (TODO doc missing)
 */
static int lui_drawstrokeparams__index(lua_State *L)
{
	lui_object *lobj = lui_checkDrawStrokeParams(L, 1);
	uiDrawStrokeParams *params = uiDrawStrokeParams(lobj->object);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "linecap") == 0) {
		lui_aux_pushNameOrValue(L, params->Cap, "lui_enumlinecap");
	} else if (strcmp(what, "linejoin") == 0) {
		lui_aux_pushNameOrValue(L, params->Cap, "lui_enumlinejoin");
	} else if (strcmp(what, "thickness") == 0) {
		lua_pushnumber(L, params->Thickness);
	} else if (strcmp(what, "miterlimit") == 0) {
		lua_pushnumber(L, params->MiterLimit);
	} else if (strcmp(what, "dashphase") == 0) {
		lua_pushnumber(L, params->DashPhase);
	} else if (strcmp(what, "dashes") == 0) {
		if (lui_aux_getUservalue(L, 1, "dashes") == LUA_TNIL) {
			DEBUGMSG("(uncached)");
			lua_pop(L, 1);
			lui_drawstrokeparams_getDashes(L, params);
			lui_aux_setUservalue(L, 1, "dashes", 3);
		}
	} else {
		return lui_utility__index(L);
	}
	return 1;
}

static int lui_drawstrokeparams__newindex(lua_State *L)
{
	lui_object *lobj = lui_checkDrawStrokeParams(L, 1);
	uiDrawStrokeParams *params = uiDrawStrokeParams(lobj->object);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "linecap") == 0) {
		params->Cap = lui_aux_getNumberOrValue(L, 3, "lui_enumlinecap");
	} else if (strcmp(what, "linejoin") == 0) {
		params->Join = lui_aux_getNumberOrValue(L, 3, "lui_enumlinejoin");
	} else if (strcmp(what, "thickness") == 0) {
		params->Thickness = luaL_checknumber(L, 3);
	} else if (strcmp(what, "miterlimit") == 0) {
		params->MiterLimit = luaL_checknumber(L, 3);
	} else if (strcmp(what, "dashphase") == 0) {
		params->DashPhase = luaL_checknumber(L, 3);
	} else if (strcmp(what, "dashes") == 0) {
		lui_drawstrokeparams_setDashes(L, params, 3);
		lua_pushnil(L);
		lui_aux_setUservalue(L, 1, "dashes", 4);
		lua_pop(L, 1);
	} else {
		return lui_utility__newindex(L);
	}
	return 0;
}

static int lui_makeDrawLineCapEnum(lua_State *L)
{
	lua_newtable(L);
	lui_enumItem("flat", uiDrawLineCapFlat);
	lui_enumItem("round", uiDrawLineCapRound);
	lui_enumItem("square", uiDrawLineCapSquare);
	return 1;
}

static int lui_makeDrawLineJoinEnum(lua_State *L)
{
	lua_newtable(L);
	lui_enumItem("miter", uiDrawLineJoinMiter);
	lui_enumItem("round", uiDrawLineJoinRound);
	lui_enumItem("bevel", uiDrawLineJoinBevel);
	return 1;
}

/*** Constructor
 * Object: draw.strokeparams
 * Name: draw.strokeparams
 * Signature: strokeparams = lui.draw.strokeparams(properties = nil)
 * create a new draw.strokeparams object
 */
static int lui_newDrawStrokeParams(lua_State *L)
{
	int hastable = lui_aux_istable(L, 1);

	lui_object *lobj = lui_pushDrawStrokeParams(L);
	lobj->object = calloc(1, sizeof(uiDrawStrokeParams));
	uiDrawStrokeParams(lobj->object)->Thickness = 1;
	uiDrawStrokeParams(lobj->object)->MiterLimit = uiDrawDefaultMiterLimit;
	if (hastable) { lui_aux_setFieldsFromTable(L, lua_gettop(L), 1); }
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for draw.strokeparams */
static const luaL_Reg lui_drawstrokeparams_meta[] = {
	{"__index", lui_drawstrokeparams__index},
	{"__newindex", lui_drawstrokeparams__newindex},
	{0, 0}
};

/* uiDrawMatrix  ***********************************************************/

/*** Object
 * Name: draw.matrix
 * a transformation matrix for drawing operations
 */
#define uiDrawMatrix(this) ((uiDrawMatrix *) (this))
#define LUI_DRAWMATRIX "lui_drawmatrix"
#define lui_pushDrawMatrix(L) lui_pushObject(L, LUI_DRAWMATRIX, 0)
#define lui_checkDrawMatrix(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_DRAWMATRIX))

/*** Method
 * Object: draw.matrix
 * Name: setidentity
 * Signature: matrix = matrix:setidentity()
 * reset the matrix to identity. Returns the changed matrix.
 */
static int lui_drawMatrixSetIdentity(lua_State *L)
{
	lui_object *lobj = lui_checkDrawMatrix(L, 1);
	uiDrawMatrixSetIdentity(uiDrawMatrix(lobj->object));
	lua_pushvalue(L, 1);
	return 1;
}

/*** Method
 * Object: draw.matrix
 * Name: translate
 * Signature: matrix = matrix:translate(x, y)
 * multiply the matrix with a matrix translating by x, y. Returns the changed
 * matrix.
 */
static int lui_drawMatrixTranslate(lua_State *L)
{
	lui_object *lobj = lui_checkDrawMatrix(L, 1);
	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	uiDrawMatrixTranslate(uiDrawMatrix(lobj->object), x, y);
	lua_pushvalue(L, 1);
	return 1;
}

/*** Method
 * Object: draw.matrix
 * Name: scale
 * Signature: matrix = matrix:scale(x, y, xcenter = 0, ycenter = 0)
 * multiply the matrix with a matrix scaling by x, y around xcenter, ycenter.
 * Returns the changed matrix.
 */
static int lui_drawMatrixScale(lua_State *L)
{
	lui_object *lobj = lui_checkDrawMatrix(L, 1);
	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	double xcenter = luaL_optnumber(L, 4, 0);
	double ycenter = luaL_optnumber(L, 5, 0);
	(void) lobj;
	uiDrawMatrixScale(uiDrawMatrix(lobj->object), xcenter, ycenter, x, y);
	lua_pushvalue(L, 1);
	return 1;
}

/*** Method
 * Object: draw.matrix
 * Name: rotate
 * Signature: matrix = matrix:rotate(x, y, angle)
 * multiply the matrix with a matrix rotating by angle around x, y. angle is
 * in radians. Returns the changed matrix.
 */
static int lui_drawMatrixRotate(lua_State *L)
{
	lui_object *lobj = lui_checkDrawMatrix(L, 1);
	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	double angle = luaL_checknumber(L, 4);
	uiDrawMatrixRotate(uiDrawMatrix(lobj->object), x, y, angle);
	lua_pushvalue(L, 1);
	return 1;
}

/*** Method
 * Object: draw.matrix
 * Name: skew
 * Signature: matrix = matrix:skew(x, y, xamount, yamount)
 * multiply the matrix with a matrix skewing around x, y by xamount, yamount
 * Returns the changed matrix.
 */
static int lui_drawMatrixSkew(lua_State *L)
{
	lui_object *lobj = lui_checkDrawMatrix(L, 1);
	(void) lobj;
	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	double xamount = luaL_checknumber(L, 4);
	double yamount = luaL_checknumber(L, 5);
	uiDrawMatrixSkew(uiDrawMatrix(lobj->object), x, y, xamount, yamount);
	lua_pushvalue(L, 1);
	return 1;
}

/*** Method
 * Object: draw.matrix
 * Name: multiply
 * Signature: matrix = matrix:multiply(matrix)
 * multiply the matrix with another matrix. Returns the changed matrix.
 */
static int lui_drawMatrixMultiply(lua_State *L)
{
	lui_object *lobj = lui_checkDrawMatrix(L, 1);
	lui_object *other = lui_checkDrawMatrix(L, 2);
	uiDrawMatrixMultiply(uiDrawMatrix(lobj->object), uiDrawMatrix(other->object));
	lua_pushvalue(L, 1);
	return 1;
}

/*** Method
 * Object: draw.matrix
 * Name: invertible
 * Signature: ok = matrix:invertible()
 * return true if the matrix is invertible, false if not.
 */
static int lui_drawMatrixInvertible(lua_State *L)
{
	lui_object *lobj = lui_checkDrawMatrix(L, 1);
	lua_pushboolean(L, uiDrawMatrixInvertible(uiDrawMatrix(lobj->object)) != 0);
	return 1;
}

/*** Method
 * Object: draw.matrix
 * Name: invert
 * Signature: ok = matrix:invert()
 * invert the matrix. Returns true if that went ok, false if not.
 */
static int lui_drawMatrixInvert(lua_State *L)
{
	lui_object *lobj = lui_checkDrawMatrix(L, 1);
	lua_pushboolean(L, uiDrawMatrixInvert(uiDrawMatrix(lobj->object)) != 0);
	return 1;
}

/*** Method
 * Object: draw.matrix
 * Name: transformpoint
 * Signature: x, y = matrix:transformpoint(x, y)
 * transforms input coordinates x, y with the matrix. Returns the transformed
 * coordinates.
 */
static int lui_drawMatrixTransformPoint(lua_State *L)
{
	lui_object *lobj = lui_checkDrawMatrix(L, 1);
	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	uiDrawMatrixTransformPoint(uiDrawMatrix(lobj->object), &x, &y);
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	return 2;
}

/*** Method
 * Object: draw.matrix
 * Name: transformsize
 * Signature: x, y = matrix:transformsize(x, y)
 * transforms input size x, y with the matrix. Returns the transformed
 * size.
 */
static int lui_drawMatrixTransformSize(lua_State *L)
{
	lui_object *lobj = lui_checkDrawMatrix(L, 1);
	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	uiDrawMatrixTransformSize(uiDrawMatrix(lobj->object), &x, &y);
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	return 2;
}

/*** Constructor
 * Object: draw.matrix
 * Name: draw.matrix
 * Signature: matrix = lui.draw.matrix()
 * creates a new draw.matrix object, initialized to identity.
 */
static int lui_newDrawMatrix(lua_State *L)
{
	lui_object *lobj = lui_pushDrawMatrix(L);
	lobj->object = calloc(1, sizeof(uiDrawMatrix));
	uiDrawMatrixSetIdentity(uiDrawMatrix(lobj->object));
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* methods for DrawMatrix */
static const luaL_Reg lui_drawmatrix_methods[] = {
	{"setidentity", lui_drawMatrixSetIdentity},
	{"translate", lui_drawMatrixTranslate},
	{"scale", lui_drawMatrixScale},
	{"rotate", lui_drawMatrixRotate},
	{"skew", lui_drawMatrixSkew},
	{"multiply", lui_drawMatrixMultiply},
	{"invertible", lui_drawMatrixInvertible},
	{"invert", lui_drawMatrixInvert},
	{"transformpoint", lui_drawMatrixTransformPoint},
	{"transformsize", lui_drawMatrixTransformSize},
	{0, 0}
};

/* uiDrawPath  *************************************************************/

/*** Object
 * Name: draw.path
 * a path to draw to a draw.context
 */
#define uiDrawPath(this) ((uiDrawPath *) (this))
#define LUI_DRAWPATH "lui_drawpath"
#define lui_pushDrawPath(L) lui_pushObject(L, LUI_DRAWPATH, 0)
#define lui_checkDrawPath(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_DRAWPATH))

static int lui_drawpath__gc(lua_State *L)
{
	lui_object *lobj = lui_checkDrawPath(L, 1);
	if (lobj->object) {
		DEBUGMSG("lui_drawpath__gc (%s)", lui_debug_controlTostring(L, 1));
		uiDrawFreePath(uiDrawPath(lobj->object));
		lobj->object = 0;
	}
	return 0;
}

/*** Method
 * Object: draw.path
 * Name: newfigure
 * Signature: path:newfigure(x, y)
 * add a new figure to a path. x, y is the coordinate where the figure starts.
 */
static int lui_drawPathNewFigure(lua_State *L)
{
	lui_object *lobj = lui_checkDrawPath(L, 1);
	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	uiDrawPathNewFigure(uiDrawPath(lobj->object), x, y);
	return 0;
}

/*** Method
 * Object: draw.path
 * Name: newfigurewitharc
 * Signature: path:newfigurewitharc(xcenter, ycenter, radius, start, sweep, negative)
 * add a new figure to a path.
 */
static int lui_drawPathNewFigureWithArc(lua_State *L)
{
	lui_object *lobj = lui_checkDrawPath(L, 1);
	double xcenter = luaL_checknumber(L, 2);
	double ycenter = luaL_checknumber(L, 3);
	double radius = luaL_checknumber(L, 4);
	double start = luaL_checknumber(L, 5);
	double sweep = luaL_checknumber(L, 6);
	int negative = lua_toboolean(L, 7);
	uiDrawPathNewFigureWithArc(uiDrawPath(lobj->object), xcenter, ycenter, radius, start, sweep, negative);
	return 0;
}

/*** Method
 * Object: draw.path
 * Name: lineto
 * Signature: path:lineto(x, y)
 * add a line to a path. A line can only be added when a figure is open.
 */
static int lui_drawPathLineTo(lua_State *L)
{
	lui_object *lobj = lui_checkDrawPath(L, 1);
	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	uiDrawPathLineTo(uiDrawPath(lobj->object), x, y);
	return 0;
}

/*** Method
 * Object: draw.path
 * Name: arcto
 * Signature: path:arcto(xcenter, ycenter, radius, start, sweep, negative = false)
 * add an arc to a path. An arc can only be added when a figure is open.
 */
static int lui_drawPathArcTo(lua_State *L)
{
	lui_object *lobj = lui_checkDrawPath(L, 1);
	double xcenter = luaL_checknumber(L, 2);
	double ycenter = luaL_checknumber(L, 3);
	double radius = luaL_checknumber(L, 4);
	double start = luaL_checknumber(L, 5);
	double sweep = luaL_checknumber(L, 6);
	int negative = lua_toboolean(L, 7);
	uiDrawPathArcTo(uiDrawPath(lobj->object), xcenter, ycenter, radius, start, sweep, negative);
	return 0;
}

/*** Method
 * Object: draw.path
 * Name: bezierto
 * Signature: path:bezierto(c1x, c1y, c2x, c2y, endx, endy)
 * add a bezier curve to a path. A bezier curve can only be added when a
 * figure is open.
 */
static int lui_drawPathBezierTo(lua_State *L)
{
	lui_object *lobj = lui_checkDrawPath(L, 1);
	double c1x = luaL_checknumber(L, 2);
	double c1y = luaL_checknumber(L, 3);
	double c2x = luaL_checknumber(L, 4);
	double c2y = luaL_checknumber(L, 5);
	double endx = luaL_checknumber(L, 6);
	double endy = luaL_checknumber(L, 7);
	uiDrawPathBezierTo(uiDrawPath(lobj->object), c1x, c1y, c2x, c2y, endx, endy);
	return 0;
}

/*** Method
 * Object: draw.path
 * Name: closefigure
 * Signature: path:closefigure()
 * close the last opened figure.
 */
static int lui_drawPathCloseFigure(lua_State *L)
{
	lui_object *lobj = lui_checkDrawPath(L, 1);
	uiDrawPathCloseFigure(uiDrawPath(lobj->object));
	return 0;
}

/*** Method
 * Object: draw.path
 * Name: addrectangle
 * Signature: path:addrectangle(x, y, width, height)
 * add a rectangle to a path. A rectangle can be added if no figure is open.
 */
static int lui_drawPathAddRectangle(lua_State *L)
{
	lui_object *lobj = lui_checkDrawPath(L, 1);
	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	double w = luaL_checknumber(L, 4);
	double h = luaL_checknumber(L, 5);
	uiDrawPathAddRectangle(uiDrawPath(lobj->object), x, y, w, h);
	return 0;
}

/*** Method
 * Object: draw.path
 * Name: done
 * Signature: path:done()
 * signal that you are finished composing the path.
 */
static int lui_drawPathEnd(lua_State *L)
{
	lui_object *lobj = lui_checkDrawPath(L, 1);
	uiDrawPathEnd(uiDrawPath(lobj->object));
	return 0;
}

/*** Constructor
 * Object: draw.path
 * Name: draw.path
 * Signature: path = lui.draw.path(fillmode = "winding")
 * create a new draw.path object. fillmode may be "winding" or "alternate",
 * or lui.enum.fillmode.winding...
 */
static int lui_newDrawPath(lua_State *L)
{
	int fillmode = uiDrawFillModeWinding;
	if (lua_gettop(L) > 0) {
		fillmode = lui_aux_getNumberOrValue(L, 1, "lui_enumfillmode");
	}
	lui_object *lobj = lui_pushDrawPath(L);
	lobj->object = uiDrawNewPath(fillmode);
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

static int lui_makeDrawFillModeEnum(lua_State *L)
{
	lua_newtable(L);
	lui_enumItem("winding", uiDrawFillModeWinding);
	lui_enumItem("alternate", uiDrawFillModeAlternate);
	return 1;
}

/* methods for DrawPath */
static const luaL_Reg lui_drawpath_methods[] = {
	{"newfigure", lui_drawPathNewFigure},
	{"newfigurewitharc", lui_drawPathNewFigureWithArc},
	{"lineto", lui_drawPathLineTo},
	{"arcto", lui_drawPathArcTo},
	{"bezierto", lui_drawPathBezierTo},
	{"closefigure", lui_drawPathCloseFigure},
	{"addrectangle", lui_drawPathAddRectangle},
	{"done", lui_drawPathEnd},
	{0, 0}
};

/* metamethods for DrawPath */
static const luaL_Reg lui_drawpath_meta[] = {
	{"__gc", lui_drawpath__gc},
	{0, 0}
};

/* uiDrawContext  **********************************************************/

/*** Object
 * Name: draw.context
 * a drawing context object, passed to area.ondraw handler
 */
#define uiDrawContext(this) ((uiDrawContext *) (this))
#define LUI_DRAWCONTEXT "lui_drawcontext"
#define lui_pushDrawContext(L) lui_pushObject(L, LUI_DRAWCONTEXT, 0)
#define lui_checkDrawContext(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_DRAWCONTEXT))

static int lui_drawcontext__gc(lua_State *L)
{
	/* nothing to do here */
	return 0;
}

/*** Method
 * Object: draw.context
 * Name: fill
 * Signature: context:fill(drawpath, brush)
 * draw a filled path. The path is filled according to the fillmode set when
 * creating the path. brush is the brush to use for filling the path.
 */
static int lui_drawContextFill(lua_State *L)
{
	lui_object *lobj = lui_checkDrawContext(L, 1);
	lui_object *path = lui_checkDrawPath(L, 2);
	lui_object *brush = lui_checkDrawBrush(L, 3);
	uiDrawFill(uiDrawContext(lobj->object), uiDrawPath(path->object), uiDrawBrush(brush->object));
	return 0;
}

/*** Method
 * Object: draw.context
 * Name: stroke
 * Signature: context:stroke(path, brush, strokeparams)
 * draw a non-filled path. brush is the brush to use for the path outline.
 * strokeparams is the description of the strokes to use for the path.
 */
static int lui_drawContextStroke(lua_State *L)
{
	lui_object *lobj = lui_checkDrawContext(L, 1);
	lui_object *path = lui_checkDrawPath(L, 2);
	lui_object *brush = lui_checkDrawBrush(L, 3);
	lui_object *sparm = lui_checkDrawStrokeParams(L, 4);
	uiDrawStroke(uiDrawContext(lobj->object), uiDrawPath(path->object), uiDrawBrush(brush->object), uiDrawStrokeParams(sparm->object));
	return 0;
}

/*** Method
 * Object: draw.context
 * Name: transform
 * Signature: context:transform(matrix)
 * transform everything that is drawn to the path according to the
 * transformations in the draw.matrix.
 */
static int lui_drawContextTransform(lua_State *L)
{
	lui_object *lobj = lui_checkDrawContext(L, 1);
	lui_object *matrix = lui_checkDrawMatrix(L, 2);	
	uiDrawTransform(uiDrawContext(lobj->object), uiDrawMatrix(matrix->object));
	return 0;
}

/*** Method
 * Object: draw.context
 * Name: clip
 * Signature: context:clip(path)
 * clip all that is rendered against the path.
 */
static int lui_drawContextClip(lua_State *L)
{
	lui_object *lobj = lui_checkDrawContext(L, 1);
	lui_object *path = lui_checkDrawPath(L, 2);
	uiDrawClip(uiDrawContext(lobj->object), uiDrawPath(path->object));
	return 0;
}

/*** Method
 * Object: draw.context
 * Name: save
 * Signature: context:save()
 * push the current drawing context state to a stack
 */
static int lui_drawContextSave(lua_State *L)
{
	lui_object *lobj = lui_checkDrawContext(L, 1);
	uiDrawSave(uiDrawContext(lobj->object));
	return 0;
}

/*** Method
 * Object: draw.context
 * Name: restore
 * Signature: context:restore()
 * restore the current drawing context state from a stack
 */
static int lui_drawContextRestore(lua_State *L)
{
	lui_object *lobj = lui_checkDrawContext(L, 1);
	uiDrawRestore(uiDrawContext(lobj->object));
	return 0;
}

/*** Method
 * Object: draw.context
 * Name: text
 * Signature: context:text(x, y, textlayout)<br>context:text(x, y, string, font, width, align = "left")
 * draw a draw.textlayout object. x, y are the positions to place the layout
 * object at. In the second form, the parameters string, font, width and
 * align are as for lui.text.layout()
 */
static int lui_drawContextText(lua_State *L)
{
	lui_object *lobj = lui_checkDrawContext(L, 1);
	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	lui_object *text = lui_toTextLayout(L, 4);
	if (text) {
		uiDrawText(uiDrawContext(lobj->object), uiDrawTextLayout(text->object), x, y);
	} else {
		uiDrawTextLayoutParams params;
		uiAttributedString *astr = NULL;
		if (lua_isstring(L, 4)) {
			astr = uiNewAttributedString(lua_tostring(L, 4));
			params.String = astr;
		} else {
			params.String = uiAttributedString(lui_checkAttributedString(L, 4)->object);
		}
		params.DefaultFont = uiFontDescriptor(lui_checkTextFont(L, 5)->object);
		params.Width = luaL_checknumber(L, 6);
		params.Align = lui_aux_getNumberOrValue(L, 7, "lui_enumtextalign");
		uiDrawTextLayout *layout = uiDrawNewTextLayout(&params);
		uiDrawText(uiDrawContext(lobj->object), layout, x, y);
		uiDrawFreeTextLayout(layout);
		if (astr) {
			uiFreeAttributedString(astr);
		}
	}
	return 0;
}

static int lui_wrapDrawcontext(lua_State *L, uiDrawContext *ctx)
{
	lui_object *lobj = lui_pushDrawContext(L);
	lobj->object = ctx;
	return 1;
}

/* metamethods for draw.context */
static const luaL_Reg lui_drawcontext_meta[] = {
	{"__gc", lui_drawcontext__gc},
	{0, 0}
};

/* methods for draw.context */
static const struct luaL_Reg lui_drawContext_methods [] ={
	{"fill", lui_drawContextFill},
	{"stroke", lui_drawContextStroke},
	{"transform", lui_drawContextTransform},
	{"clip", lui_drawContextClip},
	{"save", lui_drawContextSave},
	{"restore", lui_drawContextRestore},
	{"text", lui_drawContextText},
	{0, 0}
};

/* draw function list table
 */
static const struct luaL_Reg lui_draw_funcs [] ={
	/* utility constructors */
	{"brush", lui_newDrawBrush},
	{"strokeparams", lui_newDrawStrokeParams},
	{"matrix", lui_newDrawMatrix},
	{"path", lui_newDrawPath},
	{0, 0}
};

static void lui_addDrawEnums(lua_State *L)
{
	int top = lua_gettop(L);
	if (lua_getfield(L, top, "enum") == LUA_TNIL) {
		lua_pop(L, 1);
		lua_newtable(L);
	}

	lui_makeDrawBrushTypeEnum(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, top + 1, "brushtype");
	lua_setfield(L, LUA_REGISTRYINDEX, "lui_enumbrushtype");

	lui_makeDrawLineCapEnum(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, top + 1, "linecap");
	lua_setfield(L, LUA_REGISTRYINDEX, "lui_enumlinecap");

	lui_makeDrawLineJoinEnum(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, top + 1, "linejoin");
	lua_setfield(L, LUA_REGISTRYINDEX, "lui_enumlinejoin");

	lui_makeDrawFillModeEnum(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, top + 1, "fillmode");
	lua_setfield(L, LUA_REGISTRYINDEX, "lui_enumfillmode");

	lua_setfield(L, top, "enum");
}

static int lui_init_draw(lua_State *L)
{
	lua_newtable(L);
	luaL_setfuncs(L, lui_draw_funcs, 0);
	lua_setfield(L, -2, "draw");

	lui_add_utility_type(L, LUI_DRAWBRUSH, 0, lui_drawbrush_meta);
	lui_add_utility_type(L, LUI_DRAWSTROKEPARAMS, 0, lui_drawstrokeparams_meta);
	lui_add_utility_type(L, LUI_DRAWMATRIX, lui_drawmatrix_methods, 0);
	lui_add_utility_type(L, LUI_DRAWPATH, lui_drawpath_methods, lui_drawpath_meta);
	lui_add_utility_type(L, LUI_DRAWCONTEXT, lui_drawContext_methods, lui_drawcontext_meta);

	lui_addDrawEnums(L);

	return 1;
}
