local menu = lui.menu("File")

local open = menu:append("Open...")
open.onclicked = function(mi, win)
	local f = lui.openfile(win)
	lui.msgbox(win, mi.text, tostring(f))
end

 menu:append("Save...", {
	onclicked = function(mi, win)
		local f = lui.savefile(win)
		lui.msgbox(win, mi.text, tostring(f))
	end
})

menu:appendseparator()
menu:appendquit()
lui.onshouldquit(function() lui.quit() end)

menu = lui.menu("Misc")

menu:appendcheckable("Checkable", {
	onclicked = function(mi, win)
		lui.msgbox(win, mi.text, tostring(mi.checked))
	end
})

menu:append("Disabled", {
	enabled = false
})

return true