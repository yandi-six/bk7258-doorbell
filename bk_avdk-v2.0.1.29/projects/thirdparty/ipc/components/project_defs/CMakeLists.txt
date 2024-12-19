set(incs ./inc)
set(srcs)
list(APPEND srcs
	src/rtc_list.c
	src/rtc_bk.c
	)

armino_component_register(SRCS "${srcs}"
	INCLUDE_DIRS "${incs}"
	PRIV_REQUIRES ""
	)
