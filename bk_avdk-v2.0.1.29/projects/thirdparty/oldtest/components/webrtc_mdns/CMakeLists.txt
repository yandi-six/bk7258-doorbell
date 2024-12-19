
set(incs ./inc)
set(srcs)
if (CONFIG_WEBRTC_MDNS)
list(APPEND srcs
	src/webrtc_mdns.c
	)
unset(c_compile_options)

list(APPEND c_compile_options "-Wno-unused-variable")
list(APPEND c_compile_options "-Wno-unused-but-set-variable")
list(APPEND c_compile_options "-Wno-int-to-pointer-cast")
list(APPEND c_compile_options "-Wno-pointer-to-int-cast")
list(APPEND c_compile_options "-Wno-format")
list(APPEND c_compile_options "-Wno-maybe-uninitialized")
list(APPEND c_compile_options "-Wno-memset-elt-size")
list(APPEND c_compile_options "-Wincompatible-pointer-types")

armino_build_set_property(C_COMPILE_OPTIONS "${c_compile_options}" APPEND)


endif()
armino_component_register(SRCS "${srcs}"
	INCLUDE_DIRS "${incs}"
	PRIV_REQUIRES project_defs lwip_intf_v2_1
	)


