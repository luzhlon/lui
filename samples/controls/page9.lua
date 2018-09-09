local draw = lui.draw
local text = lui.text

local white = draw.brush { color = {r = 1, g = 1, b = 1, a = 1} }
local color = {r = 0, g = 0, b = 0, a = 1}
local strokep = {linecap = "flat", linejoin = "miter", thickness = 1, miterlimit = 10}
local filled = false
local angle = 0
local font = lui.fontbutton().font

area = lui.area()

function redraw()
	area:forceredraw()
end

function print_centered(ctx, x, y, w, h, str)
	local layout = text.layout(str, font, w)
	ctx:text(x + (w - layout.width) / 2, y + (h - layout.height) / 2, layout)
end

area.onmouse = function(area, which, button, x, y, count, modifiers, areaw, areah)
	if which ~= "down" then return end
	local aw3, ah3 = areaw / 3, areah / 3
	if x > aw3 and x < 2*aw3 and y > ah3 and y < 2*ah3 then
		area:beginuserwindowmove()
	else
		local edge
		if y < ah3 then
			if x < aw3 then
				edge = "topleft"
			elseif x < 2*aw3 then
				edge = "top"
			else
				edge = "topright"
			end
		elseif y < 2*ah3 then
			if x < aw3 then
				edge = "left"
			else
				edge = lui.enum.edge.right
			end
		else
			if x < aw3 then
				edge = lui.enum.edge.bottomleft
			elseif x < 2*aw3 then
				edge = lui.enum.edge.bottom
			else
				edge = lui.enum.edge.bottomright
			end
		end
		area:beginuserwindowresize(edge)
	end
end

area.ondraw = function(area, ctx, x, y, w, h, aw, ah)
	local path = draw.path()
	path:addrectangle(0, 0, aw, ah)
	path:done()
	ctx:fill(path, white)

	local brush = draw.brush { color = color }
	local stroke = draw.strokeparams(strokep)

	local aw3, ah3 = aw / 3, ah / 3

	path = draw.path()
	path:addrectangle(0, 0, aw3, ah3)
	path:addrectangle(aw3, 0, aw3, ah3)
	path:addrectangle(2*aw3, 0, aw3, ah3)
	path:addrectangle(0, ah3, aw3, ah3)
	path:addrectangle(aw3, ah3, aw3, ah3)
	path:addrectangle(2*aw3, ah3, aw3, ah3)
	path:addrectangle(0, 2*ah3, aw3, ah3)
	path:addrectangle(aw3, 2*ah3, aw3, ah3)
	path:addrectangle(2*aw3, 2*ah3, aw3, ah3)
	path:done()
	ctx:stroke(path, brush, stroke)

	print_centered(ctx, 0, 0, aw3, ah3, "Click here to drag top left corner")
	print_centered(ctx, aw3, 0, aw3, ah3, "Click here to drag top edge")
	print_centered(ctx, 2*aw3, 0, aw3, ah3, "Click here to drag top right corner")

	print_centered(ctx, 0, ah3, aw3, ah3, "Click here to drag left edge")
	print_centered(ctx, aw3, ah3, aw3, ah3, "Click here to move window")
	print_centered(ctx, 2*aw3, ah3, aw3, ah3, "Click here to drag right edge")

	print_centered(ctx, 0, 2*ah3, aw3, ah3, "Click here to drag bottom left corner")
	print_centered(ctx, aw3, 2*ah3, aw3, ah3, "Click here to drag bottom edge")
	print_centered(ctx, 2*aw3, 2*ah3, aw3, ah3, "Click here to drag bottom right corner")
end

return group("Window changing area", area)
