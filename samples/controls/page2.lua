local vbox = lui.vbox { padded = true }

vbox:append(lui.label("Enter secret value"))
local hbox = vbox:append(lui.hbox())
local pwe = hbox:append(lui.passwordentry(), true)
hbox:append(lui.button("Reveal", {
	onclicked = function(b)
		lui.msgbox(win, "Secret value", tostring(pwe.text))
	end
}))

vbox:append(lui.label("Search now..."))
local sbe = vbox:append(lui.searchentry())
local stxt = vbox:append(lui.label(""))
sbe.onchanged = function(e)
	stxt.text = "Searching " .. sbe.text
end

vbox:append(lui.hseparator())

local text1 = "Enter first value"
local label1 = lui.label(text1)
vbox:append(label1)
local entry1 = lui.entry {
	onchanged = function(e)
		label1.text = text1
		if e.text ~= "" then
			label1.text = label1.text .. " (" .. e.text .. ")"
		end
	end
}
vbox:append(entry1)

vbox:append(lui.label("Enter second value"))
local entry2 = vbox:append(lui.entry {
	text = "you don't enter nothing here",
	readonly = true
})

local hbox = lui.hbox()
	hbox:append(lui.button("Copy", {
		onclicked = function() entry2.text = "Copied " .. entry1.text end
	}))

	hbox:append(lui.button("Confirm", {
		onclicked = function() lui.msgbox(win, "Confirmed", entry1.text .. "\n" .. entry2.text) end
	}))
vbox:append(hbox)

local text2 = "Enter multiline value"
local label2 = lui.label(text2 .. " (0 chars)")
vbox:append(label2)
local mle = vbox:append(lui.multilineentry(false, {
	onchanged = function(m)
		label2.text = text2 .. " (" .. tostring(#m.text) .. " chars)"
	end
}), true)

hbox = lui.hbox()
	hbox:append(lui.button("Append", {
		onclicked = function() mle:append(entry1.text .. "\n") end
	}))

	hbox:append(lui.button("Clear", {
		onclicked = function() mle.text = "" end
	}))

	hbox:append(lui.button("Toggle Readonly", {
		onclicked = function() mle.readonly = not mle.readonly end
	}))
vbox:append(hbox)

return group("Entries", vbox)