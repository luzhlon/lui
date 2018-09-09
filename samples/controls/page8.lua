local draw = lui.draw
local text = lui.text

hbox = lui.hbox { padded = true }
vbox = lui.vbox()

local astr = text.attributedstring("123456789 This is some sample text to test text rendering.")

local color = {r = 1, g = 1, b = 1, a = 1}
local font = lui.fontbutton().font -- default font

local area = lui.area()

function redraw()
	area:forceredraw()
end

vbox:append(lui.colorbutton {
	onchanged = function(c)
		color = c.color
		redraw()
	end
})

vbox:append(lui.colorbutton {
	onchanged = function(c)
		astr:setattributes( { color = c.color } )
		redraw()
	end
})

vbox:append(lui.fontbutton {
	onchanged = function(f)
		font = f.font
		redraw()
	end
})

hbox:append(vbox)

area.ondraw = function(area, ctx, x, y, w, h, aw, ah)
	local brush = draw.brush { color = color }
	local path = draw.path()
	path:addrectangle(0, 0, aw, ah)
	path:done()
	ctx:fill(path, brush)
	local layout = text.layout(astr, font, aw)
	ctx:text(1, 1, layout)
	ctx:text(1, (1 + font.size) * 2, astr, font, aw)
	ctx:text(1, (1 + font.size) * 4, "and this is just some plain text. As you'll see, it is not affected by any color changes.", font, aw)
end
hbox:append(area, true)

return group("Area with text", hbox)
