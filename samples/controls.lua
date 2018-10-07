require "testing_c_path"

lui = require "lui"

function group(title, control)
	local grp = lui.group(title)
		grp.margined = true
		grp:setchild(control)
	return grp
end

local function file_exists(file)
	local f = io.open(file, "r")
	if f then
		f:close()
		return true
	end
	return false
end

local function newenv()
	return setmetatable({}, {__index = _ENV})
end

lui.init()

local func, err = loadfile("controls/menu.lua", "t", newenv())
if err then
	error(err)
end
local hasmenu = func()

win = lui.window("Controls Example", 800, 600, hasmenu, {
	onclosing = function() lui.quit() end,
	visible = true
})
local function settitle(win)
	local w, h = win:contentsize()
	win.title = string.format("Controls Example %dx%d", w, h)
end
win.oncontentsizechanged = function(win) settitle(win) end
settitle(win)

local tab = lui.tab()
win:setchild(tab)

local count = 1
local func, err = loadfile("controls/page"..tostring(count)..".lua", "t", newenv())

while func do
	print("adding controls/page"..tostring(count)..".lua")
	local control = func()
	tab:append("Page "..tostring(count), control)
	count = count + 1
	func, err = loadfile("controls/page"..tostring(count)..".lua", "t", newenv())
end
if err and file_exists("controls/page"..tostring(count)..".lua") then
	error(err)
end

local mle = tab:append("Control Tree", lui.multilineentry(true))

local function dumptree(ctl, pfx)
	pfx = pfx or ""
	mle:append(pfx .. "+-- " .. tostring(ctl) .. "\n")
	for c = 1, ctl:numchildren() do
		dumptree(ctl:getchild(c), pfx .. "|   ")
	end
end

dumptree(win)

lui.main()

-- the following is optional as it is called when lua terminates. It needs
-- to work properly when called, though
lui.finalize()
