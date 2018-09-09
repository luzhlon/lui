local vbox = lui.vbox { padded = true }

local l1 = vbox:append(lui.label(""))
local pb = vbox:append(lui.progressbar())
local function setlabel()
	l1.text = string.format("This displays some progress (%d%%)", pb.value)
end

vbox:append(lui.label("Spin this"))
local sp = vbox:append(lui.spinbox(0, 100, {
	onchanged = function(s)
		pb.value = s.value
		setlabel()
	end
}))

vbox:append(lui.label("Slide this"))
local sl = vbox:append(lui.slider(0, 100, {
	onchanged = function(s)
		pb.value = s.value
		sp.value = s.value
		setlabel()
	end
}))

return group("Sliders etc", vbox)