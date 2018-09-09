-- qrun.vim@nlua:
-- require "testing_c_path"
lui = require "lui"

lui.init()

win = lui.window("Table Test", {
	onclosing = function() lui.quit() return true end,
	visible = true
})

data = {
	{"Zeile 1", 10, "Edit 1", true, "Button 1", { some = "table" }},
	{"Zeile 2", 20, "Edit 2", true, "Button 2", "Nobutton 2"},
	{"Zeile 3", 30, "Edit 3", true, "Button 3", "Nobutton 3"},
	{"Zeile 4", 40, "Edit 4", true, "Button 4", "Nobutton 4"},
	{"Zeile 5", 50, "Edit 5", true, "Button 5", "Nobutton 5"},
	{"Zeile 6", 60, "Edit 6", true, "Button 6", "Nobutton 6"},
	{"Zeile 7", 70, "Edit 7", true, "Button 7", "Nobutton 7"},
	{"Zeile 8", 80, "Edit 8", true, "Button 8", "Nobutton 8"},
	{"Zeile 9", 90, "Edit 9", true, "Button 9", "Nobutton 9"},
	{"Zeile 10", 100, "Edit 10", true, "Button 10", "Nobutton 10"},
}

function iscolor(t)
	for k, _ in pairs(t) do
		if k ~= 'r' and k ~= 'g' and k ~= 'b' and k ~= 'a' then
			return false
		end
	end
	return true
end

function tabletype(val)
	local t = type(val)
	if t == "number" then
		print(math.type(val))
		if math.type(val) == "integer" then
			return "integer"
		else
			return "string"
		end
	elseif t == "table" and iscolor(val) then
		return "color"
	elseif t == "userdata" and string.match(tostring(t), "^lui_image:") then
		return "image"
	elseif t == "boolean" then
		return "boolean"
	else
		return "string"
	end
end

mdl = lui.tablemodel {
	numcolumns = function() return #data[1] end,
	columntype = function(col) return tabletype(data[1][col]) end,
	numrows	= function() return #data end,
	cellvalue = function(row, col) return data[row][col] end,
	setcellvalue = function(row, col, val) if val ~= nil then data[row][col] = val end end,
}

vb = win:setchild(lui.vbox(), true)
tbl = vb:append(lui.table(mdl), true)

tbl:appendtextcolumn("Text", 1)
tbl:appendprogressbarcolumn("Number", 2)
tbl:appendtextcolumn("Edit", 3, true)
tbl:appendcheckboxcolumn("Check", 4, true)
tbl:appendcheckboxcolumn("Nocheck", 4, false)
tbl:appendbuttoncolumn("Click", 5, true)
tbl:appendbuttoncolumn("Noclick", 6, false)

lui.main()
lui.finalize()

for _, r in ipairs(data) do
	for _, c in ipairs(r) do
		io.write(tostring(c), "\t")
	end
	print()
end
