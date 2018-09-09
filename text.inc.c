/* uiFontDescriptor *********************************************************/

/*** Object
 * Name: text.font
 * a font to draw text into an area with.
 */
#define uiFontDescriptor(this) ((uiFontDescriptor *) (this))
#define LUI_TEXTFONT "lui_font"
#define lui_pushTextFont(L) lui_pushObject(L, LUI_TEXTFONT, 0)
#define lui_checkTextFont(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_TEXTFONT))

static int lui_textfont__gc(lua_State *L)
{
	lui_object *lobj = lui_checkTextFont(L, 1);
	if (lobj->object) {
		DEBUGMSG("lui_textfont__gc (%s)", lui_debug_controlTostring(L, 1));
		uiFreeFontButtonFont(uiFontDescriptor(lobj->object));
		lobj->object = 0;
	}
	return 0;
}

/*** Property
 * Object: text.font
 * Name: family
 * Font family. This is a read-only property.
 *** Property
 * Object: text.font
 * Name: size
 * Font size. This is a read-only property.
 *** Property
 * Object: text.font
 * Name: weight
 * Font weight. This is a read-only property.
 *** Property
 * Object: text.font
 * Name: italic
 * Font italic-ness. This is a read-only property.
 *** Property
 * Object: text.font
 * Name: stretch
 * Font stretch. This is a read-only property.
 */
static int lui_textfont__index(lua_State *L)
{
	lui_object *lobj = lui_checkTextFont(L, 1);
	uiFontDescriptor *font = uiFontDescriptor(lobj->object);
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "family") == 0) {
		lua_pushstring(L, font->Family);
	} else if (strcmp(what, "size") == 0) {
		lua_pushnumber(L, font->Size);
	} else if (strcmp(what, "weight") == 0) {
		lui_aux_pushNameOrValue(L, font->Weight, "lui_enumtextweight");
	} else if (strcmp(what, "italic") == 0) {
		lui_aux_pushNameOrValue(L, font->Italic, "lui_enumtextitalic");
	} else if (strcmp(what, "stretch") == 0) {
		lui_aux_pushNameOrValue(L, font->Stretch, "lui_enumtextstretch");
	} else {
		return lui_utility__index(L);
	}
	return 1;
}

static int lui_wrapTextFont(lua_State *L, uiFontDescriptor *font)
{
	lui_object *lobj = lui_pushTextFont(L);
	lobj->object = font;
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/* metamethods for textfont */
static const luaL_Reg lui_textfont_meta[] = {
	{"__index", lui_textfont__index},
	{"__gc", lui_textfont__gc},
	{0, 0}
};

/* uiAttributedString *****************************************************/

/*** Object
 * Name: text.attributedstring
 * a string with formatting attributes, that can be used to create a
 * textlayout object
 */
#define uiAttributedString(this) ((uiAttributedString *) (this))
#define LUI_ATTRIBUTEDSTRING "lui_attributedstring"
#define lui_pushAttributedString(L) lui_pushObject(L, LUI_ATTRIBUTEDSTRING, 0)
#define lui_checkAttributedString(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_ATTRIBUTEDSTRING))

/*** Property
 * Object: text.attributedstring
 * Name: text
 * A string representing the textual content of the attributedstring. Note
 * that this string will not reflect the contents of the atttributedstring
 * any more if insert, delete or the append operator has been used on that.
 * This is a read-only property.
 */
static int lui_attributedstring__index(lua_State *L)
{
	uiAttributedString *astr = lui_checkAttributedString(L, 1)->object;
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "text") == 0) {
		lua_pushstring(L, uiAttributedStringString(astr));
	} else {
		return lui_utility__index(L);
	}
	return 1;
}

static int lui_attributedstring__gc(lua_State *L)
{
	lui_object *lobj = lui_checkAttributedString(L, 1);
	if (lobj->object) {
		DEBUGMSG("lui_attributedstring__gc (%s)", lui_debug_controlTostring(L, 1));
		uiFreeAttributedString(uiAttributedString(lobj->object));
		lobj->object = 0;
	}
	return 0;
}

struct uiAttributeList {
	uiAttribute *attr;
	size_t start, end;
	struct uiAttributeList *next;
};

static struct uiAttributeList *uiNewAttributeList()
{
	return calloc(1, sizeof(struct uiAttributeList));
}

static void uiFreeAttributeList(struct uiAttributeList *alst)
{
	while (alst) {
		struct uiAttributeList *nxt = alst->next;
		free(alst);
		alst = nxt;
	}
}

static struct uiAttributeList *uiAttributeListAppend(struct uiAttributeList *alst, uiAttribute *a, size_t start, size_t end)
{
	struct uiAttributeList *alste = uiNewAttributeList();
	alste->attr = a;
	alste->start = start;
	alste->end = end;
	alste->next = 0;
	if (alst) {
		struct uiAttributeList *iter = alst;
		while (iter->next) {
			iter = iter->next;
		}
		iter->next = alste;
		return alst;
	}
	return alste;
}

static uiForEach lui_AttributedStringForEachAttributeFunc(const uiAttributedString *s, const uiAttribute *a, size_t start, size_t end, void *data)
{
	struct uiAttributeList **palst = (struct uiAttributeList **) data;
	*palst = uiAttributeListAppend(*palst, (uiAttribute *)a, start, end);
	return uiForEachContinue;
}

static void lui_copyAttributes(uiAttributedString *from, uiAttributedString *to, size_t ofs)
{
	struct uiAttributeList *alst = 0;
	uiAttributedStringForEachAttribute(from, lui_AttributedStringForEachAttributeFunc, &alst);
	struct uiAttributeList *iter = alst;
	while (iter) {
		uiAttributedStringSetAttribute(to, iter->attr, iter->start + ofs, iter->end + ofs);
		iter = iter->next;
	}
	uiFreeAttributeList(alst);
}

/*** Method
 * Object: text.attributedstring
 * Name: append
 * Signature: astr = astr:append(other)
 * appends a string to an attributedstring. other may be a lua string, in
 * which case it is appended without attributes, or an attributedstring, in
 * which case the attributes are copied as well. This modifies the
 * attributedstring this method was called on. Returns the attributedstring
 * the operation was called on.
 */
static int lui_attributedStringAppend(lua_State *L)
{
	uiAttributedString *astr = lui_checkAttributedString(L, 1)->object;
	if (lua_type(L, 2) == LUA_TSTRING) {
		const char *str = lua_tostring(L, 2);
		uiAttributedStringAppendUnattributed(astr, str);
	} else {
		uiAttributedString *astr2 = lui_checkAttributedString(L, 2)->object;
		size_t ofs = uiAttributedStringLen(astr);
		uiAttributedStringAppendUnattributed(astr, uiAttributedStringString(astr2));
		lui_copyAttributes(astr2, astr, ofs);
	}
	lua_pushvalue(L, 1);
	return 1;
}

/*** Method
 * Object: text.attributedstring
 * Name: .. (concatenation operator)
 * Signature: astr = astr .. other
 * returns a new attributedstring that is the concatenation of the left and
 * the right side of the .. operator.
 */
static int lui_attributedstring__concat(lua_State *L)
{
	uiAttributedString *astr = 0;
	if (lua_isstring(L, 1)) {
		const char *str = lua_tostring(L, 1);
		astr = uiNewAttributedString(str);
	} else {
		uiAttributedString *astr1 = lui_checkAttributedString(L, 1)->object;
		astr = uiNewAttributedString(uiAttributedStringString(astr1));
		lui_copyAttributes(astr1, astr, 0);
	}
	if (lua_isstring(L, 2)) {
		const char *str = lua_tostring(L, 2);
		uiAttributedStringAppendUnattributed(astr, str);
	} else {
		uiAttributedString *astr2 = lui_checkAttributedString(L, 2)->object;
		size_t ofs = uiAttributedStringLen(astr);
		uiAttributedStringAppendUnattributed(astr, uiAttributedStringString(astr2));
		lui_copyAttributes(astr2, astr, ofs);
	}
	lui_object *lobj = lui_pushAttributedString(L);
	lobj->object = astr;
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/*** Method
 * Object: text.attributedstring
 * Name: len
 * Signature: length = astr:len()
 * returns the length of the attributedstring in bytes. len is also available
 * via the length operator (#)
 */
static int lui_attributedStringLen(lua_State *L)
{
	lui_object *lobj = lui_checkAttributedString(L, 1);
	lua_pushinteger(L, uiAttributedStringLen(lobj->object));
	return 1;
}

static int lui_attributedstring__len(lua_State *L)
{
	return lui_attributedStringLen(L);
}

/*** Method
 * Object: text.attributedstring
 * Name: numchars
 * Signature: num = astr:numchars()
 * returns the number of visible chars in a string. A visible char is a
 * single renderable unit, a.k.a a grapheme.
 */
static int lui_attributedStringNumChars(lua_State *L)
{
	uiAttributedString *astr = lui_checkAttributedString(L, 1)->object;
	lua_pushinteger(L, uiAttributedStringNumGraphemes(astr));
	return 1;
}

/*** Method
 * Object: text.attributedstring
 * Name: chartobytepos
 * Signature: bytepos = astr:chartobytepos(charpos)
 * converts a byte position to a visible char position in an attributedstring.
 */
static int lui_attributedStringCharToBytePos(lua_State *L)
{
	uiAttributedString *astr = lui_checkAttributedString(L, 1)->object;
	int cpos = luaL_checkinteger(L, 2) - 1;
	lua_pushinteger(L, uiAttributedStringGraphemeToByteIndex(astr, cpos) + 1);
	return 1;
}

/*** Method
 * Object: text.attributedstring
 * Name: bytetocharpos
 * Signature: charpos = astr:bytetocharpos(bytepos)
 * converts a visible char position to a byte position in an attributedstring.
 */
static int lui_attributedStringByteToCharPos(lua_State *L)
{
	uiAttributedString *astr = lui_checkAttributedString(L, 1)->object;
	int bpos = luaL_checkinteger(L, 2) - 1;
	lua_pushinteger(L, uiAttributedStringByteIndexToGrapheme(astr, bpos) + 1);
	return 1;
}

/*** Method
 * Object: text.attributedstring
 * Name: insert
 * Signature: astr = astr:insert(other, pos)
 * inserts another string into an attributedstring at position pos. other
 * may be a lua string, in which case it is appended without attributes,
 * or an attributedstring, in which case the attributes are copied as well.
 * This modifies the attributedstring this method was called on. Returns
 * the attributedstring the operation was called on.
 */
static int lui_attributedStringInsert(lua_State *L)
{
	uiAttributedString *astr = lui_checkAttributedString(L, 1)->object;
	int pos = luaL_checkinteger(L, 3) - 1;
	if (lua_type(L, 2) == LUA_TSTRING) {
		const char *str = lua_tostring(L, 2);
		uiAttributedStringInsertAtUnattributed(astr, str, pos);
	} else {
		uiAttributedString *astr2 =lui_checkAttributedString(L, 2)->object;
		uiAttributedStringInsertAtUnattributed(astr, uiAttributedStringString(astr2), pos);
		lui_copyAttributes(astr2, astr, pos);
	}
	lua_pushvalue(L, 1);
	return 1;
}

/*** Method
 * Object: text.attributedstring
 * Name: delete
 * Signature: astr = astr:delete(start, end)
 * deletes all bytes between (and including) start and end from the
 * attributedstring. This modifies the attributedstring this method was
 * called on. Returns the attributedstring the operation was called on.
 * Note: this function is currently broken in libui, and thus also in lui.
 */
static int lui_attributedStringDelete(lua_State *L)
{
	uiAttributedString *astr = lui_checkAttributedString(L, 1)->object;
	int start = luaL_checkinteger(L, 2) - 1;
	int end = luaL_optinteger(L, 3, uiAttributedStringLen(astr) + 1); // end is inclusive here!
	uiAttributedStringDelete(astr, start, end);
	lua_pushvalue(L, 1);
	return 1;
}

static int lui_makeTextWeightEnum(lua_State *L)
{
	lua_newtable(L);
	lui_enumItem("minimum", uiTextWeightMinimum);
	lui_enumItem("thin", uiTextWeightThin);
	lui_enumItem("ultralight", uiTextWeightUltraLight);
	lui_enumItem("light", uiTextWeightLight);
	lui_enumItem("book", uiTextWeightBook);
	lui_enumItem("normal", uiTextWeightNormal);
	lui_enumItem("medium", uiTextWeightMedium);
	lui_enumItem("semibold", uiTextWeightSemiBold);
	lui_enumItem("bold", uiTextWeightBold);
	lui_enumItem("ultrabold", uiTextWeightUltraBold);
	lui_enumItem("heavy", uiTextWeightHeavy);
	lui_enumItem("ultraheavy", uiTextWeightUltraHeavy);
	lui_enumItem("maximum", uiTextWeightMaximum);
	return 1;
}

static int lui_makeTextItalicEnum(lua_State *L)
{
	lua_newtable(L);
	lui_enumItem("normal", uiTextItalicNormal);
	lui_enumItem("oblique", uiTextItalicOblique);
	lui_enumItem("italic", uiTextItalicItalic);
	return 1;
}

static int lui_makeTextStretchEnum(lua_State *L)
{
	lua_newtable(L);
	lui_enumItem("ultracondensed", uiTextStretchUltraCondensed);
	lui_enumItem("extracondensed", uiTextStretchExtraCondensed);
	lui_enumItem("condensed", uiTextStretchCondensed);
	lui_enumItem("semicondensed", uiTextStretchSemiCondensed);
	lui_enumItem("normal", uiTextStretchNormal);
	lui_enumItem("semiexpanded", uiTextStretchSemiExpanded);
	lui_enumItem("expanded", uiTextStretchExpanded);
	lui_enumItem("extraexpanded", uiTextStretchExtraExpanded);
	lui_enumItem("ultraexpanded", uiTextStretchUltraExpanded);
	return 1;
}

static int lui_makeTextUnderlineEnum(lua_State *L)
{
	lua_newtable(L);
	lui_enumItem("none", uiUnderlineNone);
	lui_enumItem("single", uiUnderlineSingle);
	lui_enumItem("double", uiUnderlineDouble);
	lui_enumItem("suggestion", uiUnderlineSuggestion);
	return 1;
}

static int lui_makeTextUnderlineColorEnum(lua_State *L)
{
	lua_newtable(L);
	//lui_enumItem("custom", uiUnderlineColorCustom); implicit by using a rgba table
	lui_enumItem("spelling", uiUnderlineColorSpelling);
	lui_enumItem("grammar", uiUnderlineColorGrammar);
	lui_enumItem("auxiliary", uiUnderlineColorAuxiliary);
	return 1;
}

static int lui_attributedStringSetAttributesFromTable(lua_State *L)
{
	uiAttributedString *astr = lui_checkAttributedString(L, 1)->object;
	int start = luaL_optinteger(L, 3, 1) - 1;
	int end = luaL_optinteger(L, 4, uiAttributedStringLen(astr) + 1); // end is inclusive here!

	lua_pushnil(L);
	while (lua_next(L, 2) != 0) {
		uiAttribute *attr = 0;
		const char *key = lua_tostring(L, -2);
		if (strcmp(key, "family") == 0) {
			const char *value = lua_tostring(L, -1);
			attr = uiNewFamilyAttribute(value);
		} else if (strcmp(key, "size") == 0) {
			double value = lua_tonumber(L, -1);
			attr = uiNewSizeAttribute(value);
		} else if (strcmp(key, "weight") == 0) {
			int value = lui_aux_getNumberOrValue(L, -1, "lui_enumtextweight");
			attr = uiNewWeightAttribute(value);
		} else if (strcmp(key, "italic") == 0) {
			int value = lui_aux_getNumberOrValue(L, -1, "lui_enumtextitalic");
			attr = uiNewItalicAttribute(value);
		} else if (strcmp(key, "stretch") == 0) {
			int value = lui_aux_getNumberOrValue(L, -1, "lui_enumtextstretch");
			attr = uiNewStretchAttribute(value);
		} else if (strcmp(key, "color") == 0) {
			double r, g, b, a;
			lui_aux_rgbaFromTable(L, -1, &r, &g, &b, &a);
			attr = uiNewColorAttribute(r, g, b, a);
		} else if (strcmp(key, "bgcolor") == 0) {
			double r, g, b, a;
			lui_aux_rgbaFromTable(L, -1, &r, &g, &b, &a);
			attr = uiNewBackgroundAttribute(r, g, b, a);
		} else if (strcmp(key, "underline") == 0) {
			int value = lui_aux_getNumberOrValue(L, -1, "lui_enumtextunderline");
			attr = uiNewUnderlineAttribute(value);
		} else if (strcmp(key, "ulcolor") == 0) {
			if (lua_type(L, -1) == LUA_TTABLE) {
				double r, g, b, a;
				lui_aux_rgbaFromTable(L, -1, &r, &g, &b, &a);
				attr = uiNewUnderlineColorAttribute(uiUnderlineColorCustom, r, g, b, a);
			} else {
				int value = lui_aux_getNumberOrValue(L, -1, "lui_enumtextulcolor");
				attr = uiNewUnderlineColorAttribute(value, 0, 0, 0, 0);
			}
		// TODO add Opentype attributes?
		} else {
			return luaL_error(L, "invalid attribute: %s", key);
		}
		if (attr) {
			uiAttributedStringSetAttribute(astr, attr, start, end);
		}
		lua_pop(L, 1);
	}
	lua_pushvalue(L, 1);
	return 1;
}

static int lui_attributedStringSetAttributesFromFont(lua_State *L)
{
	uiAttributedString *astr = lui_checkAttributedString(L, 1)->object;
	lui_object *fobj = lui_checkTextFont(L, 2);
	uiFontDescriptor *font = uiFontDescriptor(fobj->object);
	int start = luaL_optinteger(L, 3, 1) - 1;
	int end = luaL_optinteger(L, 4, uiAttributedStringLen(astr) + 1); // end is inclusive here!

	uiAttribute *attr = uiNewFamilyAttribute(font->Family);
	uiAttributedStringSetAttribute(astr, attr, start, end);
	attr = uiNewSizeAttribute(font->Size);
	uiAttributedStringSetAttribute(astr, attr, start, end);
	attr = uiNewWeightAttribute(font->Weight);
	uiAttributedStringSetAttribute(astr, attr, start, end);
	attr = uiNewItalicAttribute(font->Italic);
	uiAttributedStringSetAttribute(astr, attr, start, end);
	attr = uiNewStretchAttribute(font->Stretch);
	uiAttributedStringSetAttribute(astr, attr, start, end);
	lua_pushvalue(L, 1);
	return 1;
}

/*** Method
 * Object: text.attributedstring
 * Name: setattributes
 * Signature: astr = astr:setattributes(attr, start = 1, end = #astr)
 * sets the attributes for a section of the string. atr may be a text.font,
 * or a table with attribute names as keys and attribute values as values.
 * start is the first byte and end is the last byte to set the attributes
 * for. This modifies the attributedstring this method was called on.
 * Returns the attributedstring the operation was called on.
 *
 * Valid attribute names and values are:
 *
 * - family: font family
 * - size: font size
 * - weight: font weight, any of the strings "minimum", "thin", "ultralight",
 *   "light", "book", "normal", "medium", "semibold", "bold", "ultrabold",
 *   "heavy", "ultraheavy", "maximum", or lui.enum.weight.minimum, ... or an
 *   integer between 0 and 1000.
 * - italic: font italicness, any of the strings "normal", "oblique",
 *   "italic", or lui.enum.italic.normal, ...
 * - stretch: font width, any of the strings "ultracondensed",
 *   "extracondensed", "condensed", "semicondensed", "normal", "semiexpanded",
 *   "expanded", "extraexpanded", "ultraexpanded", or
 *   lui.enum.stretch.ultracondensed, ...
 * - color: foreground color, a table { r = red, g = green, b = blue,
 *   a = alpha }, where the values range from 0.0 to 1.0
 * - bgcolor: background color, value as for color.
 * - underline: underline style, any of he strings "none", "single", "double",
 *   "suggestion", 
 * - ulcolor: underline color, any of the strings "spelling", "grammar",
 *   "auxiliary", or a rgba table as for color.
 */
static int lui_attributedStringSetAttributes(lua_State *L)
{
	int t = lua_type(L, 2);
	if (t == LUA_TTABLE) {
		return lui_attributedStringSetAttributesFromTable(L);
	} else if (t == LUA_TUSERDATA) {
		return lui_attributedStringSetAttributesFromFont(L);
	}
	return luaL_error(L, "invalid type for attributes, expected table or font");
}

static int lui_pushAttributeNameAndValue(lua_State *L, uiAttribute *attr)
{
	switch (uiAttributeGetType(attr)) {
		case uiAttributeTypeFamily:
			lua_pushstring(L, "family");
			lua_pushstring(L, uiAttributeFamily(attr));
			break;
		case uiAttributeTypeSize:
			lua_pushstring(L, "size");
			lua_pushnumber(L, uiAttributeSize(attr));
			break;
		case uiAttributeTypeWeight:
			lua_pushstring(L, "weight");
			lui_aux_pushNameOrValue(L, uiAttributeWeight(attr), "lui_enumtextweight");
			break;
		case uiAttributeTypeItalic:
			lua_pushstring(L, "italic");
			lui_aux_pushNameOrValue(L, uiAttributeItalic(attr), "lui_enumtextitalic");
			break;
		case uiAttributeTypeStretch:
			lua_pushstring(L, "stretch");
			lui_aux_pushNameOrValue(L, uiAttributeStretch(attr), "lui_enumtextstretch");
			break;
		case uiAttributeTypeColor:
			{
				double r, g, b, a;
				lua_pushstring(L, "color");
				uiAttributeColor(attr, &r, &g, &b, &a);
				lui_aux_pushRgbaAsTable(L, r, g, b, a);
			}
			break;
		case uiAttributeTypeBackground:
			{
				double r, g, b, a;
				lua_pushstring(L, "bgcolor");
				uiAttributeColor(attr, &r, &g, &b, &a);
				lui_aux_pushRgbaAsTable(L, r, g, b, a);
			}
			break;
		case uiAttributeTypeUnderline:
			lua_pushstring(L, "underline");
			lui_aux_pushNameOrValue(L, uiAttributeUnderline(attr), "lui_enumtextunderline");
			break;
		case uiAttributeTypeUnderlineColor:
			{
				double r, g, b, a;
				uiUnderlineColor u;
				lua_pushstring(L, "ulcolor");
				uiAttributeUnderlineColor(attr, &u, &r, &g, &b, &a);
				if (u != uiUnderlineColorCustom) {
					lui_aux_pushNameOrValue(L, u, "lui_enumtextulcolor");
				} else {
					lui_aux_pushRgbaAsTable(L, r, g, b, a);
				}
			}
			break;
		//case uiAttributeTypeFeatures:
		default:
			DEBUGMSG("unknnown attribute type: %d", uiAttributeGetType(attr));
			return 0;
	}
	return 2;
}

/*** Method
 * Object: text.attributedstring
 * Name: getattributes
 * Signature: attrlist = astr:getattributes(start = nil, end = start)
 * returns a list of all active attributes. If start and end are positive
 * integers, then only those attributes are returned, that are active
 * between start and end in the attributedstring. If start is a positive
 * integer and end is omitted, then the attributes at position start are
 * returned. If both start and end are omittted, all attributes of the string
 * are returned. The return value is a table whose entries are tables of the
 * form { attr, start, end }, where attr, start and end are the same as the
 * arguments to setattributes.
 */
// TODO output is weird sometimes, investigate
static int lui_attributedStringGetAttributes(lua_State *L)
{
	uiAttributedString *astr = uiAttributedString(lui_checkAttributedString(L, 1)->object);
	int len = uiAttributedStringLen(astr);
	int spos = luaL_optinteger(L, 2, 0);
	int epos = luaL_optinteger(L, 2, spos);
	struct uiAttributeList *alst = 0;
	uiAttributedStringForEachAttribute(astr, lui_AttributedStringForEachAttributeFunc, &alst);
	struct uiAttributeList *iter = alst;
	int start = 0, end = 0, cstart = 0, cend = 0, idx = 1;
	lua_newtable(L);
	while (iter) {
		start = iter->start + 1;
		end = iter->end;
		if (start > len) {
			start = len;
		}
		if (end > len) {
			end = len;
		}
		// DEBUGMSG("%d\t%d\n", start,  end);
		if (start <= end && (!spos || (start <= epos && spos <= end))) {
			if (cstart != start || cend != end) {
				if (cstart) {
					lua_rawseti(L, -2, 1);
					lua_pushinteger(L, cstart);
					lua_rawseti(L, -2, 2);
					lua_pushinteger(L, cend);
					lua_rawseti(L, -2, 3);
					lua_rawseti(L, -2, idx++);
				}
				cstart = start;
				cend = end;
				start = 0;
				end = 0;
				lua_newtable(L); // row
				lua_newtable(L); // attr
			}
			lui_pushAttributeNameAndValue(L, iter->attr);
			lua_settable(L, -3);
		}
		iter = iter->next;
	}
	if (start) {
		lua_rawseti(L, -2, 1);
		lua_pushinteger(L, cstart);
		lua_rawseti(L, -2, 2);
		lua_pushinteger(L, cend);
		lua_rawseti(L, -2, 3);
		lua_rawseti(L, -2, idx);
	}
	return 1;
}

/* metamethods for attributedstring */
static const luaL_Reg lui_attributedstring_meta[] = {
	{"__concat", lui_attributedstring__concat},
	{"__len", lui_attributedstring__len},
	{"__index", lui_attributedstring__index},
	{"__gc", lui_attributedstring__gc},
	{0, 0}
};

/* methods for attributedstring */
static const struct luaL_Reg lui_attributedstring_methods [] ={
	{"len", lui_attributedStringLen},
	{"numchars", lui_attributedStringNumChars},
	{"bytetocharpos", lui_attributedStringByteToCharPos},
	{"chartobytepos", lui_attributedStringCharToBytePos},
	{"append", lui_attributedStringAppend},
	{"insert", lui_attributedStringInsert},
	{"delete", lui_attributedStringDelete},
	{"setattributes", lui_attributedStringSetAttributes},
	{"getattributes", lui_attributedStringGetAttributes},
	{0, 0}
};

/*** Constructor
 * Object: text.attributedstring
 * Name: text.attributedstring
 * Signature: attributedstring = lui.text.attributedstring(string = "")
 * create a new text.attributedstring object
 */
static int lui_newAttributedString(lua_State *L)
{
	const char *str = luaL_optstring(L, 1, "");
	lui_object *lobj = lui_pushAttributedString(L);
	lobj->object = uiNewAttributedString(str);
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

/*** Object
 * Name: text.layout
 * a font to draw text into an area with.
 */
#define uiDrawTextLayout(this) ((uiDrawTextLayout *) (this))
#define LUI_TEXTLAYOUT "lui_textlayout"
#define lui_pushTextLayout(L) lui_pushObject(L, LUI_TEXTLAYOUT, 0)
#define lui_toTextLayout(L, pos) ((lui_object*)luaL_testudata(L, pos, LUI_TEXTLAYOUT))
#define lui_checkTextLayout(L, pos) ((lui_object*)luaL_checkudata(L, pos, LUI_TEXTLAYOUT))

static int lui_textlayout__gc(lua_State *L)
{
	lui_object *lobj = lui_checkTextLayout(L, 1);
	if (lobj->object) {
		DEBUGMSG("lui_textlayout__gc (%s)", lui_debug_controlTostring(L, 1));
		uiDrawFreeTextLayout(uiDrawTextLayout(lobj->object));
		lobj->object = 0;
	}
	return 0;
}

/*** Property
 * Object: text.layout
 * Name: width
 * the width of the textlayout object
 * This is a read-only property.
 *** Property
 * Object: text.layout
 * Name: height
 * the height of the textlayout object
 * This is a read-only property.
 */
static int lui_textlayout__index(lua_State *L)
{
	uiDrawTextLayout *layout = lui_checkTextLayout(L, 1)->object;
	const char *what = luaL_checkstring(L, 2);

	if (strcmp(what, "width") == 0) {
		double w, h;
		uiDrawTextLayoutExtents(layout, &w, &h);
		lua_pushnumber(L, w);
	} else if (strcmp(what, "height") == 0) {
		double w, h;
		uiDrawTextLayoutExtents(layout, &w, &h);
		lua_pushnumber(L, h);
	} else {
		return lui_utility__index(L);
	}
	return 1;
}

/* metamethods for attributedstring */
static const luaL_Reg lui_textlayout_meta[] = {
	{"__gc", lui_textlayout__gc},
	{"__index", lui_textlayout__index},
	{0, 0}
};

static int lui_makeTextAlignEnum(lua_State *L)
{
	lua_newtable(L);
	lui_enumItem("left", uiDrawTextAlignLeft);
	lui_enumItem("center", uiDrawTextAlignCenter);
	lui_enumItem("right", uiDrawTextAlignRight);
	return 1;
}

/*** Constructor
 * Object: text.layout
 * Name: text.layout
 * Signature: brush = lui.text.layout(astr, font, width, align = "left")
 * create a new text.layout object. astr may be a plain string or an
 * attributedstring. font is the default font to use. width is the rendering
 * width for the string. align is the alignment, may be any of "left",
 * "center" or "right".
 */
static int lui_newTextLayout(lua_State *L)
{
	uiDrawTextLayoutParams params;
	params.DefaultFont = lui_checkTextFont(L, 2)->object;
	params.Width = luaL_checknumber(L, 3);
	params.Align = lui_aux_getNumberOrValue(L, 4, "lui_enumtextalign");
	int astrpos = 1;
	if (lua_isstring(L, 1)) {
		lui_object *lobj = lui_pushAttributedString(L);
		lobj->object = uiNewAttributedString(lua_tostring(L, 1));
		params.String = lobj->object;
		astrpos = lua_gettop(L);
		lui_registerObject(L, astrpos);
	} else {
		params.String = lui_checkAttributedString(L, 1)->object;
	}
	lui_object *lobj = lui_pushTextLayout(L);
	lobj->object = uiDrawNewTextLayout(&params);
	lua_pushvalue(L, astrpos);
	lua_setuservalue(L, -2);
	lui_registerObject(L, lua_gettop(L));
	return 1;
}

static void lui_addTextEnums(lua_State *L)
{
	int top = lua_gettop(L);
	if (lua_getfield(L, top, "enum") == LUA_TNIL) {
		lua_pop(L, 1);
		lua_newtable(L);
	}

	lui_makeTextWeightEnum(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, top + 1, "weight");
	lua_setfield(L, LUA_REGISTRYINDEX, "lui_enumtextweight");

	lui_makeTextItalicEnum(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, top + 1, "italic");
	lua_setfield(L, LUA_REGISTRYINDEX, "lui_enumtextitalic");

	lui_makeTextStretchEnum(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, top + 1, "stretch");
	lua_setfield(L, LUA_REGISTRYINDEX, "lui_enumtextstretch");

	lui_makeTextUnderlineEnum(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, top + 1, "underline");
	lua_setfield(L, LUA_REGISTRYINDEX, "lui_enumtextunderline");

	lui_makeTextUnderlineColorEnum(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, top + 1, "ulcolor");
	lua_setfield(L, LUA_REGISTRYINDEX, "lui_enumtextulcolor");

	lui_makeTextAlignEnum(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, top + 1, "align");
	lua_setfield(L, LUA_REGISTRYINDEX, "lui_enumtextalign");

	lua_setfield(L, top, "enum");
}

/* text function list table
 */
static const struct luaL_Reg lui_text_funcs [] ={
	/* utility constructors */
	{"attributedstring", lui_newAttributedString},
	{"layout", lui_newTextLayout},
	{0, 0}
};


static int lui_init_text(lua_State *L)
{
	lua_newtable(L);
	luaL_setfuncs(L, lui_text_funcs, 0);
	lua_setfield(L, -2, "text");

	lui_add_utility_type(L, LUI_TEXTFONT, 0, lui_textfont_meta);
	lui_add_utility_type(L, LUI_ATTRIBUTEDSTRING, lui_attributedstring_methods, lui_attributedstring_meta);
	lui_add_utility_type(L, LUI_TEXTLAYOUT, 0, lui_textlayout_meta);

	lui_addTextEnums(L);

	return 1;
}

