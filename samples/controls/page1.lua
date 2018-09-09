local vbox = lui.vbox()

local function button_callback(b, title)
	lui.msgbox(win, title or "Clicked button", b.text)
end

local btn = lui.button("Press me first")
	btn.onclicked = button_callback
vbox:append(btn, true)

btn = lui.button("Press me next")
	btn.onclicked = button_callback
vbox:append(btn, true)

vbox:append(lui.button("Press me finally", {
	text = "Press me last",
	onclicked = setmetatable({ title = "Private title" }, {__call = function(t, c) button_callback(c, t.title) end})
}), true)

vbox:append(lui.hseparator())
vbox:append(lui.button("Crash now", {
	onclicked = function() error("CRASH") end
}))

return group("Buttons", vbox)