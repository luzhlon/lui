local config = {}
string.gsub(package.config, "%C", function(s) config[#config+1] = s end)
package.cpath = ".." .. config[1] .. config[3] .. ".so" .. config[2] .. package.cpath
