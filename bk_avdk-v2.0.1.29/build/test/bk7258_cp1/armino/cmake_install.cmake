# Install script for directory: /home/yan/work/doolbell/bk_avdk-v2.0.1.29/bk_idk

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
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/part_table/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/partitions/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk7258_cp1/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/common/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/release/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/base64/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/easy_flash/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/arm9/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/riscv/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_rtos/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/os_source/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/coredump/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/cmsis/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/cm33/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_log/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/mbedtls/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_bluetooth/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_phy/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_ate/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/driver/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/psa_mbedtls/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/tfm_mbedtls/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/wolfssl/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/wpa_supplicant-2.10/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/user_driver/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_adapter/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/uart_debug/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_wifi/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/lwip_intf_v2_1/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_netif/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_libs/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_init/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/embeded_gcov/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_startup/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/utf8/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/temp_detect/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_system/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/iperf/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_usb/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/fatfs/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_ps/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/ali_mqtt/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bt/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/at/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/unity/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/compal/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_https/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/ota/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/http/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/key/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/littlefs/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_vfs/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/jrtplib/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/flashdb/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_cli/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/app/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_event/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_common/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/include/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/adc_key/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/at_server/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_airkiss/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/mesh/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_httpc/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_pm/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_uid/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_websocket/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/compress/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/touch/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/os/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/hmac_sha_256/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/json/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/key_handle/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/micropython/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/openssl/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/qcbor/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/saradc_intf/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/security/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/webclient/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/vnd_flash/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/avdk_libs/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/bk_peripheral/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/project_defs/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/webrtc_minor/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/webrtc_avi/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/webrtc_jpeg/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/webrtc_major/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/webrtc_mdns/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/webrtc_mp4/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/webrtc_sdk/cmake_install.cmake")
  include("/home/yan/work/doolbell/bk_avdk-v2.0.1.29/build/test/bk7258_cp1/armino/main/cmake_install.cmake")

endif()
