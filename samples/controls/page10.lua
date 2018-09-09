vbox = lui.vbox()

vbox:append(lui.button("Errorbox", {
	onclicked = function()
		lui.errorbox(win, "Error", "Kaputt")
	end
}))

vbox:append(lui.button("Messagebox", {
	onclicked = function()
		lui.msgbox(win, "Message", "Nachricht")
	end
}))

vbox:append(lui.button("Open Dialog", {
	onclicked = function()
		local res = lui.openfile(win)
		lui.msgbox(win, "Open Dialog", "open dialog returned " .. tostring(res))
	end
}))

vbox:append(lui.button("Save Dialog", {
	onclicked = function()
		local res = lui.savefile(win)
		lui.msgbox(win, "Open Dialog", "open dialog returned " .. tostring(res))
	end
}))

vbox:append(lui.button("Custom Dialog", {
	onclicked = function()
		local form = lui.form()
		local vn_entry = form:append("First", lui.entry())
		local nn_entry = form:append("Second", lui.entry())
		local ok = lui.dialog("Custom Dialog", 400, 300, form, "OK", "Cancel", "Other")
		lui.msgbox(win, "Custom Dialog", "custom dialog returned " .. tostring(ok) .. "\nFirst: " .. vn_entry.text .. "\nSecond: " .. nn_entry.text)
	end
}))

return group("Dialogs", vbox)
