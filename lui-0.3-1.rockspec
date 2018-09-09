package = "lui"
version = "0.3-1"
source = {
   url = "http://www.tset.de/downloads/lui-0.3-1.tar.gz"
}
description = {
   summary = "A gui library for lua that uses native UI elements on all supported platforms",
   detailed = [[
		libui.
	]],
   homepage = "http://www.tset.de/lui/",
   license = "MIT",
   maintainer = "Gunnar Zötl <gz@tset.de>"
}
supported_platforms = {
   "unix"
}
dependencies = {
   "lua >= 5.1, <= 5.3"
}
build = {
   type = "make",
   copy_directories = {
      "doc",
      "samples"
   },
   build_variables = {
      CFLAGS = "$(CFLAGS)",
      LIBFLAG = "$(LIBFLAG)",
      LUA = "$(LUA)",
      LUA_BINDIR = "$(LUA_BINDIR)",
      LUA_INCDIR = "$(LUA_INCDIR)",
      LUA_LIBDIR = "$(LUA_LIBDIR)"
   },
   install_variables = {
      INST_CONFDIR = "$(CONFDIR)",
      INST_LIBDIR = "$(LIBDIR)",
      INST_LUADIR = "$(LUADIR)",
      INST_PREFIX = "$(PREFIX)"
   }
}
