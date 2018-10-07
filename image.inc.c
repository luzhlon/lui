/* uiImage ******************************************************************/

/*** Object
 * Name: image
 * an image
 */
#define uiImage(this) ((uiImage *) (this))
#define LUI_IMAGE "lui_image"
#define lui_pushImage(L) lui_pushObject(L, LUI_IMAGE, 0)
#define lui_checkImage(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_IMAGE))

static int lui_image__gc(lua_State *L)
{
	lui_object *lobj = lui_checkImage(L, 1);
	if (lobj->object) {
		DEBUGMSG("lui_image__gc (%s)", lui_debug_controlTostring(L, 1));
		uiFreeImage(uiImage(lobj->object));
		lobj->object = 0;
	}
	return 0;
}

// TODO __index for width and height?

/* metamethods for image */
static const luaL_Reg lui_image_meta[] = {
	{"__gc", lui_image__gc},
	{0, 0}
};

// TODO should this be append? See libui/test/page16.c, appendImageNamed()
/*** Method
 * Object: image
 * Name: append
 * Signature: image = image:append(width, height, pixels)
 * set the pixels of an image. pixels is a table of either width * height * 4
 * float values in the order r, g, b, a for each pixel, or width * height rgba
 * tables as used with ui.draw.
 * Returns the image this method was called upon.
 */
static int lui_image_append(lua_State *L)
{
	lui_object *lobj = lui_checkImage(L, 1);
	(void) lobj; // TODO
	//uiImageAppend(uiImage(lobj->object), pixels, width, height, width * 4);
	// check what happens if a representation with same width / height is appended to an image
	//lua_pushvalue(L, 1);
	//return 1;
	return luaL_error(L, "not implemented");
}

/* methods for image */
static const luaL_Reg lui_image_methods[] = {
	{"append", lui_image_append},
	{0, 0}
};

/*** Constructor
 * Object: image
 * Name: image
 * Signature: image = lui.image(width, height)
 * create a new, empty image object. width and height are the respective
 * sizes of the image in points. Typically this is the size in pixels.
 */
static int lui_newImage(lua_State *L)
{
	double width = luaL_checknumber(L, 1);
	double height = luaL_checknumber(L, 2);
	lui_object *lobj = lui_pushImage(L);
	lobj->object = uiNewImage(width, height);
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/*** Constructor
 * Object: image
 * Name: loadimage
 * Signature: image = lui.loadimage(filename)
 * create a new image object by loading it from a file. Width and height are
 * set from the respective sizes of the image file.
 */
static int lui_loadImage(lua_State *L)
{
	const char *file = luaL_checkstring(L, 1);
	(void) file; // TODO
	return luaL_error(L, "not implemented");
}

/* image function list table
 */
static const struct luaL_Reg lui_image_funcs [] ={
	/* utility constructors */
	{"image", lui_newImage},
	{"loadimage", lui_loadImage},
	{0, 0}
};

static int lui_init_image(lua_State *L)
{
	luaL_setfuncs(L, lui_image_funcs, 0);

	lui_add_utility_type(L, LUI_IMAGE, lui_image_methods, lui_image_meta);

	return 1;
}
