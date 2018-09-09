local draw = lui.draw

hbox = lui.hbox { padded = true }
vbox = lui.vbox { padded = true }

local white = draw.brush { color = {r = 1, g = 1, b = 1, a = 1} }
local color = {r = 0, g = 0, b = 0, a = 1}
local strokep = {linecap = "flat", linejoin = "miter", thickness = 1, miterlimit = 10}
local filled = false
local angle = 0

area = lui.area()

function redraw()
	area:forceredraw()
end

vbox:append(lui.checkbox("Filled", {
	ontoggled = function(c)
		filled = c.checked
		redraw()
	end
}))

vbox:append(lui.colorbutton {
	onchanged = function(c)
		color = c.color
		redraw()
	end
})

vbox:append(lui.label("line cap"))
local cbcopts = { "flat", "round", "square" }
local cbc = vbox:append(lui.combobox {
	onselected = function(c)
		strokep.linecap = cbcopts[c.selected]
		redraw()
	end
})
cbc:append(table.unpack(cbcopts))
cbc.selected = 1

vbox:append(lui.label("line join"))
local cbjopts = { "miter", "round", "bevel" }
local cbj = vbox:append(lui.combobox {
	onselected = function(c)
		strokep.linejoin = cbjopts[c.selected]
		redraw()
	end
})
cbj:append(table.unpack(cbjopts))
cbj.selected = 1

vbox:append(lui.label("line thickness"))
vbox:append(lui.slider(1, 50, {
	value = strokep.thickness,
	onchanged = function(s)
		strokep.thickness = s.value
		redraw()
	end
}))

vbox:append(lui.label("miter limit"))
vbox:append(lui.slider(1, 50, {
	value = strokep.miterlimit,
	onchanged = function(s)
		strokep.miterlimit = s.value
		redraw()
	end
}))

vbox:append(lui.label("rotation angle"))
vbox:append(lui.slider(0, 359, {
	value = angle,
	onchanged = function(s)
		angle = s.value
		redraw()
	end
}))

hbox:append(vbox)

area.ondraw = function(area, ctx, x, y, w, h, aw, ah)
	local path = draw.path()
	path:addrectangle(0, 0, aw, ah)
	path:done()
	ctx:fill(path, white)

	local brush = draw.brush { color = color }
	local stroke = draw.strokeparams(strokep)

	path = draw.path()
	path:newfigure(aw / 8, 0)
	path:lineto(aw / 2, 0)
	path:lineto(aw / 2, ah / 2)
	path:lineto(0, ah / 2)
	path:lineto(0, ah / 8)
	path:done()

	local matrix = draw.matrix()
	matrix:translate(aw / 4, ah / 4)
	matrix:rotate(aw / 4, ah / 4, angle / 180 * math.pi)
	ctx:transform(matrix)
	if filled then
		ctx:fill(path, brush)
	else
		ctx:stroke(path, brush, stroke)
	end

end
hbox:append(area, true)

return group("Area with graphics", hbox)
