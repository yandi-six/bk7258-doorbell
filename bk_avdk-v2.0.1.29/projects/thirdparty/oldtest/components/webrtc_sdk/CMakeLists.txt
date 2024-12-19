if (CONFIG_WEBRTC_SDK)

armino_build_get_property(armino_soc ARMINO_SOC)

set(incs include)
unset(c_compile_options)
list(APPEND c_compile_options "-DMBEDTLS_SSL_DTLS_SRTP")
list(APPEND c_compile_options "-Wno-unused-variable")
list(APPEND c_compile_options "-Wno-unused-but-set-variable")
armino_build_set_property(C_COMPILE_OPTIONS "${c_compile_options}" APPEND)
endif()

armino_component_register(INCLUDE_DIRS "${incs}")

if (CONFIG_WEBRTC_SDK)
	file(GLOB webrtc_libs ${CMAKE_CURRENT_SOURCE_DIR}/lib/*.a)
	foreach(lib ${webrtc_libs})
		get_filename_component(lib_name ${lib} NAME)
		add_prebuilt_library(${lib_name} "${CMAKE_CURRENT_SOURCE_DIR}/lib/${lib_name}")
		target_link_libraries(${COMPONENT_LIB} INTERFACE ${lib_name})
	endforeach()
endif()
