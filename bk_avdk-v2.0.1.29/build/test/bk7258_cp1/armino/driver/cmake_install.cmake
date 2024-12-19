# Install script for directory: /home/yan/work/doolbell/bk_avdk-v2.0.1.29/bk_idk/middleware/driver

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/driver/h264/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/driver/device/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/driver/hw_rotate/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/driver/yuv_buf/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/driver/hw_scale/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/driver/lin/cmake_install.cmake")

endif()

