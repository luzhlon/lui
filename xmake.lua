
set_project 'lui'

target 'lui'
    set_kind 'shared'

    add_includedirs 'libui'
    add_includedirs 'D:/git/nlua/src'
    add_linkdirs 'D:/git/nlua/build/debug/x86'
    add_linkdirs 'libui'

    add_files 'lui.c'

    add_links 'lua53'
    add_links 'libui'
