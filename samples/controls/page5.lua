local form = lui.form { padded = true }

local entr1 = form:append("First entry", lui.entry())
local entr2 = form:append("Second entry", lui.entry())
local entr3 = form:append("Third entry", lui.spinbox(1, 10))

return group("Form container", form)