local grid = lui.grid()

grid:append(lui.button("1"), 1, 1, 1, 1, true, "fill", true, "fill")
grid:append(lui.button("2"), 2, 1, 1, 1, true, "fill", true, "fill")
grid:append(lui.button("3"), 1, 2, 2, 1, true, "fill", true, "fill")
local c = grid:append(lui.button("4"), 1, 3, 1, 1, true, "fill", true, "fill")
grid:append(lui.button("6"), 1, 4, 2, 1, true, "fill", true, "fill")
local x = grid:insertat(lui.button("5a"), c, "leading", 1, 1, true, "fill", true, "fill")
grid:insertat(lui.button("5b"), c, "trailing", 1, 1, true, "fill", true, "fill")
grid:insertat(lui.button("5c"), x, "top", 1, 1, true, "fill", true, "fill")
grid:insertat(lui.button("5d"), x, "bottom", 1, 1, true, "fill", true, "fill")
grid:append(lui.label("Top Left"), 0, 1, 1, 1, true, "center", true, "center")

return group("Grid", grid)
