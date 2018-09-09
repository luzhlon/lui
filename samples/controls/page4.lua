local vbox = lui.vbox { padded = true }

vbox:append(lui.label("Actions show here"))
local entry = vbox:append(lui.entry { readonly = true })

local function toggled(c)
	entry.text = c.text .. " is now " .. (c.checked and "checked" or "unchecked")
end

vbox:append(lui.hseparator())

vbox:append(lui.label("Check it out"))
local hbox = lui.hbox()
	local ck1 = hbox:append(lui.checkbox("First item", { ontoggled = toggled }))
	local ck2 = hbox:append(lui.checkbox("Second item", { ontoggled = toggled }))
	local ck3 = hbox:append(lui.checkbox("Third item", { ontoggled = toggled }))
vbox:append(hbox)

vbox:append(lui.hseparator())

local items = { "option 1", "option 2", "option 3" }

vbox:append(lui.label("Combo Combo"))
local cbb = vbox:append(lui.combobox())
cbb:append(table.unpack(items))
cbb.onselected = function(c)
	entry.text = "combobox selected #" .. tostring(c.selected) .. " -> " .. c.text
end

vbox:append(lui.label("Combo DIY"))
local ebb = vbox:append(lui.editablecombobox())
for _, item in ipairs(items) do
	ebb:append(item)
end
ebb.onchanged = function(e)
	entry.text = "editable combobox selected " .. tostring(e.text)
end

vbox:append(lui.hseparator())

vbox:append(lui.label("Radio"))
local rb = vbox:append(lui.radiobuttons {
	onselected = function(r)
		lui.msgbox(win, "Radio", "selected #" .. tostring(r.selected) .. " -> " .. r.text)
	end
})
rb:append(table.unpack(items))
rb.selected = #items

vbox:append(lui.hseparator())

vbox:append(lui.label("Date/Time pickers"))
local function dtp_onchanged(d)
	local txt = ""
	txt = txt .. "day: " .. d.day .. "\n"
	txt = txt .. "mon: " .. d.mon .. "\n"
	txt = txt .. "year: " .. d.year .. "\n"
	txt = txt .. "hour: " .. d.hour .. "\n"
	txt = txt .. "min: " .. d.min .. "\n"
	txt = txt .. "sec: " .. d.sec .. "\n"
	txt = txt .. "date: "
	for k, v in pairs(d.date) do txt = txt .. k .. "=" .. v .. " " end
	txt = txt .. "\n"
	txt = txt .. "time: "
	for k, v in pairs(d.time) do txt = txt .. k .. "=" .. v .. " " end
	txt = txt .. "\n"
	txt = txt .. "datetime: "
	for k, v in pairs(d.datetime) do txt = txt .. k .. "=" .. v .. " " end
	txt = txt .. "\n"
	lui.msgbox(win, "DateTimePicker", txt)
end
local dtp = vbox:append(lui.datetimepicker { onchanged = dtp_onchanged })
local dp = vbox:append(lui.datepicker { onchanged = dtp_onchanged })
local tp = vbox:append(lui.timepicker { onchanged = dtp_onchanged })

return group("Misc controls", vbox)